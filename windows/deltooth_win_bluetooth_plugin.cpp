#include "deltooth_win_bluetooth_plugin.h"

// This must be included before many other Windows headers.
#include <windows.h>

// For getPlatformVersion; remove unless needed for your plugin implementation.
#include <VersionHelpers.h>

// WinRT / C++/WinRT headers
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Devices.Enumeration.h>
#include <winrt/Windows.Devices.Bluetooth.h>
#include <winrt/Windows.Devices.Bluetooth.Advertisement.h>
#include <winrt/Windows.Devices.Bluetooth.GenericAttributeProfile.h>
#include <winrt/Windows.Devices.Bluetooth.Rfcomm.h>
#include <winrt/Windows.Storage.Streams.h>

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Devices;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Devices::Bluetooth;
using namespace Windows::Devices::Bluetooth::Advertisement;
using namespace Windows::Devices::Bluetooth::GenericAttributeProfile;
using namespace Windows::Devices::Bluetooth::Rfcomm;

#include <flutter/method_channel.h>
#include <flutter/event_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <memory>
#include <sstream>

namespace deltooth_win_bluetooth {

// static
void DeltoothWinBluetoothPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrarWindows *registrar) {
  auto channel =
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          registrar->messenger(), "deltooth_win_bluetooth",
          &flutter::StandardMethodCodec::GetInstance());

  auto plugin = std::make_unique<DeltoothWinBluetoothPlugin>();

  channel->SetMethodCallHandler(
      [plugin_pointer = plugin.get()](const auto &call, auto result) {
        plugin_pointer->HandleMethodCall(call, std::move(result));
      });

  auto event_channel =
      std::make_unique<flutter::EventChannel<flutter::EncodableValue>>(
          registrar->messenger(), "deltooth_win_bluetooth/scan",
          &flutter::StandardMethodCodec::GetInstance());

  auto handler = std::make_unique<flutter::StreamHandlerFunctions<flutter::EncodableValue>>(
      [plugin_pointer](const flutter::EncodableValue *arguments,
                       std::unique_ptr<flutter::EventSink<flutter::EncodableValue>> &&events)
          -> std::unique_ptr<flutter::StreamHandlerError<flutter::EncodableValue>> {
        plugin_pointer->StartListening(std::move(events));
        return nullptr;
      },
      [plugin_pointer](const flutter::EncodableValue *arguments)
          -> std::unique_ptr<flutter::StreamHandlerError<flutter::EncodableValue>> {
        plugin_pointer->StopListening();
        return nullptr;
      });
  event_channel->SetStreamHandler(std::move(handler));

  registrar->AddPlugin(std::move(plugin));
}

DeltoothWinBluetoothPlugin::DeltoothWinBluetoothPlugin() {}

DeltoothWinBluetoothPlugin::~DeltoothWinBluetoothPlugin() {}

void DeltoothWinBluetoothPlugin::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue> &method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  if (method_call.method_name().compare("getPlatformVersion") == 0) {
    std::ostringstream version_stream;
    version_stream << "Windows ";
    if (IsWindows10OrGreater()) {
      version_stream << "10+";
    } else if (IsWindows8OrGreater()) {
      version_stream << "8";
    } else if (IsWindows7OrGreater()) {
      version_stream << "7";
    }
    result->Success(flutter::EncodableValue(version_stream.str()));
  } else if (method_call.method_name().compare("startScan") == 0) {
    try {
      init_apartment(apartment_type::multi_threaded);

      auto selector = BluetoothLEDevice::GetDeviceSelector();
      auto async = DeviceInformation::FindAllAsync(selector);
      auto infos = async.get();

      flutter::EncodableList devices;
      for (auto const &info : infos) {
        flutter::EncodableMap device_map;
        device_map[flutter::EncodableValue("id")] =
            flutter::EncodableValue(to_string(info.Id()));
        device_map[flutter::EncodableValue("name")] =
            flutter::EncodableValue(to_string(info.Name()));
        devices.emplace_back(std::move(device_map));
      }
      result->Success(flutter::EncodableValue(devices));
    } catch (const winrt::hresult_error &e) {
      result->Error(std::to_string(e.code().value), to_string(e.message()));
    }
  } else if (method_call.method_name().compare("startScanStream") == 0) {
    StartWatcher();
    result->Success();
  } else if (method_call.method_name().compare("stopScanStream") == 0) {
    StopWatcher();
    result->Success();
  } else {
    result->NotImplemented();
  }
}

void DeltoothWinBluetoothPlugin::StartListening(
    std::unique_ptr<flutter::EventSink<flutter::EncodableValue>> &&sink) {
  scan_sink_ = std::move(sink);
  StartWatcher();
}

void DeltoothWinBluetoothPlugin::StopListening() {
  StopWatcher();
  scan_sink_.reset();
}

void DeltoothWinBluetoothPlugin::StartWatcher() {
  try {
    init_apartment(apartment_type::multi_threaded);
  } catch (...) {
  }

  if (!watcher_) {
    watcher_ = BluetoothLEAdvertisementWatcher();
    watcher_.ScanningMode(BluetoothLEScanningMode::Active);
  }
  if (watcher_token_.value == 0) {
    watcher_token_ = watcher_.Received({this, &DeltoothWinBluetoothPlugin::OnAdvertisement});
  }
  watcher_.Start();
  if (scan_sink_) {
    flutter::EncodableMap evt;
    evt[flutter::EncodableValue("event")] = flutter::EncodableValue("started");
    scan_sink_->Success(flutter::EncodableValue(evt));
  }
}

void DeltoothWinBluetoothPlugin::StopWatcher() {
  if (watcher_) {
    watcher_.Stop();
    if (watcher_token_.value != 0) {
      watcher_.Received(watcher_token_);
      watcher_token_ = {};
    }
  }
  if (scan_sink_) {
    flutter::EncodableMap evt;
    evt[flutter::EncodableValue("event")] = flutter::EncodableValue("stopped");
    scan_sink_->Success(flutter::EncodableValue(evt));
  }
}

void DeltoothWinBluetoothPlugin::OnAdvertisement(
    BluetoothLEAdvertisementWatcher const& /*sender*/,
    BluetoothLEAdvertisementReceivedEventArgs const& args) {
  if (!scan_sink_) return;
  flutter::EncodableMap m;
  // Formatar endereço como AA:BB:CC:DD:EE:FF
  auto addr = args.BluetoothAddress();
  std::stringstream ss;
  ss << std::uppercase << std::hex << std::setw(12) << std::setfill('0') << addr;
  std::string hex = ss.str();
  std::string mac;
  for (int i = 0; i < 12; i += 2) {
    if (!mac.empty()) mac += ":";
    mac += hex.substr(i, 2);
  }
  m[flutter::EncodableValue("id")] = flutter::EncodableValue(mac);
  auto name = args.Advertisement().LocalName();
  if (name.size() > 0) {
    m[flutter::EncodableValue("name")] = flutter::EncodableValue(to_string(name));
  } else {
    m[flutter::EncodableValue("name")] = flutter::EncodableValue("");
  }
  m[flutter::EncodableValue("rssi")] = flutter::EncodableValue(static_cast<int>(args.RawSignalStrengthInDBm()));
  scan_sink_->Success(flutter::EncodableValue(m));
}

}  // namespace deltooth_win_bluetooth
