import 'deltooth_win_bluetooth_platform_interface.dart';
import 'package:flutter/services.dart';

class DeltoothWinBluetooth {
  static const _ch = MethodChannel('deltooth_win_bluetooth');
  static const _scan = EventChannel('deltooth_win_bluetooth/scan');

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

  Future<Map<String, dynamic>> connect(int address) async {
    final res = await _ch.invokeMethod('connect', {'address': address});
    return (res as Map).cast<String, dynamic>();
    // se seu endereço vier como string "AA:BB:..", troque o tipo acima
  }

  Future<bool> disconnect(int address) async {
    final res = await _ch.invokeMethod('disconnect', {'address': address});
    return res == true;
  }

  Future<List<Map<String, dynamic>>> getConnected() async {
    final res = await _ch.invokeMethod('getConnected');
    return (res as List)
        .map((e) => (e as Map).cast<String, dynamic>())
        .toList();
  }

  Future<List<String>> getServices(int address) async {
    final res = await _ch.invokeMethod('getServices', {'address': address});
    return (res as List).map((e) => (e as Map)['uuid'].toString()).toList();
  }

  Future<List<String>> getCharacteristics(
      int address, String serviceUuid) async {
    final res = await _ch.invokeMethod('getCharacteristics', {
      'address': address,
      'service': serviceUuid,
    });
    return (res as List).map((e) => (e as Map)['uuid'].toString()).toList();
  }
}
