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
#include <iomanip>

namespace deltooth_win_bluetooth {

// Stream handler concreto para o EventChannel de scan
class ScanStreamHandler final
    : public flutter::StreamHandler<flutter::EncodableValue> {
 public:
  explicit ScanStreamHandler(DeltoothWinBluetoothPlugin* plugin)
      : plugin_(plugin) {}

 protected:
  std::unique_ptr<flutter::StreamHandlerError<flutter::EncodableValue>>
  OnListenInternal(
      const flutter::EncodableValue* /*arguments*/,
      std::unique_ptr<flutter::EventSink<flutter::EncodableValue>>&& events)
      override {
    plugin_->StartListening(std::move(events));
    return nullptr;
  }

  std::unique_ptr<flutter::StreamHandlerError<flutter::EncodableValue>>
  OnCancelInternal(const flutter::EncodableValue* /*arguments*/) override {
    plugin_->StopListening();
    return nullptr;
  }

 private:
  DeltoothWinBluetoothPlugin* plugin_;
};

// static
void DeltoothWinBluetoothPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrarWindows *registrar) {
  auto channel =
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          registrar->messenger(), "deltooth_win_bluetooth",
          &flutter::StandardMethodCodec::GetInstance());

  auto plugin = std::make_unique<DeltoothWinBluetoothPlugin>();
  auto plugin_raw = plugin.get();

  channel->SetMethodCallHandler(
      [plugin_pointer = plugin.get()](const auto &call, auto result) {
        plugin_pointer->HandleMethodCall(call, std::move(result));
      });

  auto event_channel =
      std::make_unique<flutter::EventChannel<flutter::EncodableValue>>(
          registrar->messenger(), "deltooth_win_bluetooth/scan",
          &flutter::StandardMethodCodec::GetInstance());

  auto handler = std::make_unique<ScanStreamHandler>(plugin_raw);
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
  if (method_call.method_name() == "connect") {
    // args: {"address": 123456789012345ULL}  (ou string “AA:BB:..” que você converte p/ uint64)
    const auto* args = std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!args || !args->count(flutter::EncodableValue("address"))) {
      result->Error("bad-args", "missing 'address'");
      return;
    }
    uint64_t addr = 0;
    const auto& v = args->at(flutter::EncodableValue("address"));
    if (const auto p = std::get_if<int64_t>(&std::get<flutter::EncodableValue>(v))) addr = static_cast<uint64_t>(*p);
    // se vier string "AA:BB:..", faça um parse pra uint64_t aqui
  
    try {
      using namespace winrt::Windows::Devices::Bluetooth;
      auto async = BluetoothLEDevice::FromBluetoothAddressAsync(addr);
      auto dev   = async.get();
      if (!dev) { result->Error("not-found", "device null"); return; }
  
      // guardar para gerenciar conexão/estado
      connected_[addr] = dev;
      dev.ConnectionStatusChanged([this, addr](auto const& d, auto const&) {
        auto st = d.ConnectionStatus();
        // opcional: emitir evento em um EventChannel de “conn”
      });
  
      flutter::EncodableMap out;
      out[flutter::EncodableValue("name")] = flutter::EncodableValue(winrt::to_string(dev.Name()));
      out[flutter::EncodableValue("connected")] =
          flutter::EncodableValue(dev.ConnectionStatus() == BluetoothConnectionStatus::Connected);
      result->Success(out);
    } catch (const winrt::hresult_error& e) {
      result->Error(std::to_string(e.code().value), winrt::to_string(e.message()));
    }
    return;
  }

  if (method_call.method_name() == "disconnect") {
    const auto* args = std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!args || !args->count(flutter::EncodableValue("address"))) {
      result->Error("bad-args", "missing 'address'"); return;
    }
    uint64_t addr = (uint64_t)std::get<int64_t>(args->at(flutter::EncodableValue("address")));
    auto it = connected_.find(addr);
    if (it != connected_.end()) {
      it->second.Close();            // encerra handle WinRT
      connected_.erase(it);
    }
    result->Success(flutter::EncodableValue(true));
    return;
  }

  if (method_call.method_name() == "getConnected") {
    flutter::EncodableList list;
    for (auto& kv : connected_) {
      flutter::EncodableMap m;
      m[flutter::EncodableValue("address")] = flutter::EncodableValue((int64_t)kv.first);
      m[flutter::EncodableValue("name")]    = flutter::EncodableValue(winrt::to_string(kv.second.Name()));
      m[flutter::EncodableValue("status")]  = flutter::EncodableValue(
          kv.second.ConnectionStatus() == winrt::Windows::Devices::Bluetooth::BluetoothConnectionStatus::Connected);
      list.emplace_back(m);
    }
    result->Success(list);
    return;
  }

  if (method_call.method_name() == "getServices") {
    const auto* args = std::get_if<flutter::EncodableMap>(method_call.arguments());
    uint64_t addr = (uint64_t)std::get<int64_t>(args->at(flutter::EncodableValue("address")));
    auto dev = connected_.at(addr);
    auto res = dev.GetGattServicesAsync().get();   // GattDeviceServicesResult
    auto list = res.Services();                    // IVectorView<GattDeviceService>
    flutter::EncodableList out;
    for (auto const& s : list) {
      auto uuid = winrt::to_string(winrt::to_hstring(s.Uuid()));
      flutter::EncodableMap m; m[flutter::EncodableValue("uuid")] = flutter::EncodableValue(uuid);
      out.emplace_back(m);
    }
    result->Success(out);
    return;
  }
  
  if (method_call.method_name() == "getCharacteristics") {
    const auto* args = std::get_if<flutter::EncodableMap>(method_call.arguments());
    uint64_t addr = (uint64_t)std::get<int64_t>(args->at(flutter::EncodableValue("address")));
    auto uuidStr   = std::get<std::string>(args->at(flutter::EncodableValue("service")));
    auto dev = connected_.at(addr);
    auto services = dev.GetGattServicesAsync().get().Services();
    flutter::EncodableList out;
    for (auto const& s : services) {
      if (winrt::to_string(winrt::to_hstring(s.Uuid())) == uuidStr) {
        auto chars = s.GetCharacteristicsAsync().get().Characteristics();
        for (auto const& c : chars) {
          auto cuuid = winrt::to_string(winrt::to_hstring(c.Uuid()));
          flutter::EncodableMap m; m[flutter::EncodableValue("uuid")] = flutter::EncodableValue(cuuid);
          out.emplace_back(m);
        }
      }
    }
    result->Success(out);
    return;
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
