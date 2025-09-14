// import 'package:flutter_test/flutter_test.dart';
// import 'package:deltooth_win_bluetooth/deltooth_win_bluetooth.dart';
// import 'package:deltooth_win_bluetooth/deltooth_win_bluetooth_platform_interface.dart';
// import 'package:deltooth_win_bluetooth/deltooth_win_bluetooth_method_channel.dart';
// import 'package:plugin_platform_interface/plugin_platform_interface.dart';

// class MockDeltoothWinBluetoothPlatform
//     with MockPlatformInterfaceMixin
//     implements DeltoothWinBluetoothPlatform {
//   @override
//   Future<String?> getPlatformVersion() => Future.value('42');

//   @override
//   Future<List<Map<String, dynamic>>> startScan() async =>
//       <Map<String, dynamic>>[];
// }

// void main() {
//   final DeltoothWinBluetoothPlatform initialPlatform =
//       DeltoothWinBluetoothPlatform.instance;

//   test('$MethodChannelDeltoothWinBluetooth is the default instance', () {
//     expect(initialPlatform, isInstanceOf<MethodChannelDeltoothWinBluetooth>());
//   });

//   test('getPlatformVersion', () async {
//     DeltoothWinBluetooth deltoothWinBluetoothPlugin = DeltoothWinBluetooth();
//     MockDeltoothWinBluetoothPlatform fakePlatform =
//         MockDeltoothWinBluetoothPlatform();
//     DeltoothWinBluetoothPlatform.instance = fakePlatform;

//     expect(await deltoothWinBluetoothPlugin.getPlatformVersion(), '42');
//   });
// }
