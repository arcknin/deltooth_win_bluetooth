#include "include/deltooth_win_bluetooth/deltooth_win_bluetooth_plugin_c_api.h"

#include <flutter/plugin_registrar_windows.h>

#include "deltooth_win_bluetooth_plugin.h"

void DeltoothWinBluetoothPluginCApiRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  deltooth_win_bluetooth::DeltoothWinBluetoothPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
