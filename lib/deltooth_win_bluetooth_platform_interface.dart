import 'package:plugin_platform_interface/plugin_platform_interface.dart';

import 'deltooth_win_bluetooth_method_channel.dart';

abstract class DeltoothWinBluetoothPlatform extends PlatformInterface {
  /// Constructs a DeltoothWinBluetoothPlatform.
  DeltoothWinBluetoothPlatform() : super(token: _token);

  static final Object _token = Object();

  static DeltoothWinBluetoothPlatform _instance =
      MethodChannelDeltoothWinBluetooth();

  /// The default instance of [DeltoothWinBluetoothPlatform] to use.
  ///
  /// Defaults to [MethodChannelDeltoothWinBluetooth].
  static DeltoothWinBluetoothPlatform get instance => _instance;

  /// Platform-specific implementations should set this with their own
  /// platform-specific class that extends [DeltoothWinBluetoothPlatform] when
  /// they register themselves.
  static set instance(DeltoothWinBluetoothPlatform instance) {
    PlatformInterface.verifyToken(instance, _token);
    _instance = instance;
  }

  Future<String?> getPlatformVersion() {
    throw UnimplementedError('platformVersion() has not been implemented.');
  }

  Future<List<Map<String, dynamic>>> startScan() {
    throw UnimplementedError('startScan() has not been implemented.');
  }

  // Stream de descobertas BLE em tempo real
  Stream<Map<String, dynamic>> scanStream() {
    throw UnimplementedError('scanStream() has not been implemented.');
  }

  Future<void> startScanStream() {
    throw UnimplementedError('startScanStream() has not been implemented.');
  }

  Future<void> stopScanStream() {
    throw UnimplementedError('stopScanStream() has not been implemented.');
  }
}
