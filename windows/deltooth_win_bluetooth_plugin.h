#ifndef FLUTTER_PLUGIN_DELTOOTH_WIN_BLUETOOTH_PLUGIN_H_
#define FLUTTER_PLUGIN_DELTOOTH_WIN_BLUETOOTH_PLUGIN_H_

#include <flutter/method_channel.h>
#include <flutter/event_channel.h>
#include <flutter/plugin_registrar_windows.h>

// WinRT headers used in member declarations
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Devices.Bluetooth.Advertisement.h>

#include <memory>

#include <map>
#include <winrt/Windows.Devices.Bluetooth.h>
std::map<uint64_t, winrt::Windows::Devices::Bluetooth::BluetoothLEDevice> connected_;

namespace deltooth_win_bluetooth {

class DeltoothWinBluetoothPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);

  DeltoothWinBluetoothPlugin();

  virtual ~DeltoothWinBluetoothPlugin();

  // Disallow copy and assign.
  DeltoothWinBluetoothPlugin(const DeltoothWinBluetoothPlugin&) = delete;
  DeltoothWinBluetoothPlugin& operator=(const DeltoothWinBluetoothPlugin&) = delete;

  // Called when a method is called on this plugin's channel from Dart.
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);

  // Expostos para o handler de stream
  void StartListening(std::unique_ptr<flutter::EventSink<flutter::EncodableValue>> &&sink);
  void StopListening();

 private:
  // Stream sink para eventos de scan
  std::unique_ptr<flutter::EventSink<flutter::EncodableValue>> scan_sink_;

  // Watcher BLE
  winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementWatcher watcher_{ nullptr };
  winrt::event_token watcher_token_{};

  void StartWatcher();
  void StopWatcher();
  void OnAdvertisement(winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementWatcher const& sender,
                       winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementReceivedEventArgs const& args);
};

}  // namespace deltooth_win_bluetooth

#endif  // FLUTTER_PLUGIN_DELTOOTH_WIN_BLUETOOTH_PLUGIN_H_
