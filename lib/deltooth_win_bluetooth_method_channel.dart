import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';

import 'deltooth_win_bluetooth_platform_interface.dart';

/// An implementation of [DeltoothWinBluetoothPlatform] that uses method channels.
class MethodChannelDeltoothWinBluetooth extends DeltoothWinBluetoothPlatform {
  /// The method channel used to interact with the native platform.
  @visibleForTesting
  final methodChannel = const MethodChannel('deltooth_win_bluetooth');
  @visibleForTesting
  final EventChannel eventChannel = const EventChannel(
    'deltooth_win_bluetooth/scan',
  );

  @override
  Future<String?> getPlatformVersion() async {
    final version = await methodChannel.invokeMethod<String>(
      'getPlatformVersion',
    );
    return version;
  }

  @override
  Future<List<Map<String, dynamic>>> startScan() async {
    final list = await methodChannel.invokeMethod<List<dynamic>>('startScan');
    if (list == null) return <Map<String, dynamic>>[];
    return list
        .map(
          (e) => Map<String, dynamic>.from((e as Map).cast<String, dynamic>()),
        )
        .toList();
  }

  Stream<Map<String, dynamic>>? _scanStream;

  @override
  Stream<Map<String, dynamic>> scanStream() {
    _scanStream ??= eventChannel.receiveBroadcastStream().map(
      (e) => Map<String, dynamic>.from((e as Map).cast<String, dynamic>()),
    );
    return _scanStream!;
  }

  @override
  Future<void> startScanStream() async {
    await methodChannel.invokeMethod<void>('startScanStream');
  }

  @override
  Future<void> stopScanStream() async {
    await methodChannel.invokeMethod<void>('stopScanStream');
  }
}
