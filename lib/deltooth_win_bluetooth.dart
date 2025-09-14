import 'deltooth_win_bluetooth_platform_interface.dart';

class DeltoothWinBluetooth {
  Future<String?> getPlatformVersion() {
    return DeltoothWinBluetoothPlatform.instance.getPlatformVersion();
  }

  Future<List<Map<String, dynamic>>> startScan() {
    return DeltoothWinBluetoothPlatform.instance.startScan();
  }

  Stream<Map<String, dynamic>> scanStream() {
    return DeltoothWinBluetoothPlatform.instance.scanStream();
  }

  Future<void> startScanStream() {
    return DeltoothWinBluetoothPlatform.instance.startScanStream();
  }

  Future<void> stopScanStream() {
    return DeltoothWinBluetoothPlatform.instance.stopScanStream();
  }
}
