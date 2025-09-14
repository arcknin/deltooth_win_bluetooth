import 'package:flutter/services.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:deltooth_win_bluetooth/deltooth_win_bluetooth_method_channel.dart';

void main() {
  TestWidgetsFlutterBinding.ensureInitialized();

  MethodChannelDeltoothWinBluetooth platform = MethodChannelDeltoothWinBluetooth();
  const MethodChannel channel = MethodChannel('deltooth_win_bluetooth');

  setUp(() {
    TestDefaultBinaryMessengerBinding.instance.defaultBinaryMessenger.setMockMethodCallHandler(
      channel,
      (MethodCall methodCall) async {
        return '42';
      },
    );
  });

  tearDown(() {
    TestDefaultBinaryMessengerBinding.instance.defaultBinaryMessenger.setMockMethodCallHandler(channel, null);
  });

  test('getPlatformVersion', () async {
    expect(await platform.getPlatformVersion(), '42');
  });
}
