# deltooth_win_bluetooth

Plugin Flutter para Windows 10/11 que expõe funcionalidades de Bluetooth Low Energy (BLE) usando as APIs WinRT (`Windows.Devices.Bluetooth`).

O foco é substituir/estender a experiência do gerenciador Bluetooth do Windows, oferecendo:
- Scan em tempo real de anúncios BLE via EventChannel
- Listagem rápida síncrona de dispositivos BLE visíveis
- Eventos de início e parada de varredura
- Base pronta para conexão GATT (próximas versões)

## Requisitos
- Windows 10 ou 11
- Visual Studio com suporte a CMake e C++/WinRT
- Flutter para Windows configurado

## Instalação
Adicione o plugin no `pubspec.yaml` do seu app (exemplo com path local durante o desenvolvimento):

```yaml
dependencies:
  deltooth_win_bluetooth:
    path: ../deltooth_win_bluetooth
```

## Uso rápido

### Listagem rápida (snapshot)
Retorna uma lista estática de dispositivos no momento da chamada.

```dart
import 'package:deltooth_win_bluetooth/deltooth_win_bluetooth.dart';

final api = DeltoothWinBluetooth();
final devices = await api.startScan();
for (final d in devices) {
  print('id=${d['id']} name=${d['name']}');
}
```

### Scan em tempo real (stream)
Recebe anúncios BLE conforme chegam. O `id` é o endereço em formato `AA:BB:CC:DD:EE:FF`. Eventos especiais são emitidos:
- `{event: "started"}` quando o watcher inicia
- `{event: "stopped"}` quando o watcher para

```dart
import 'package:deltooth_win_bluetooth/deltooth_win_bluetooth.dart';

final api = DeltoothWinBluetooth();

await api.startScanStream();
final sub = api.scanStream().listen((event) {
  if (event['event'] == 'started' || event['event'] == 'stopped') {
    print('scan ${event['event']}');
    return;
  }
  print('BLE: ${event['name']} (${event['id']}) RSSI=${event['rssi']}');
});

// ... quando terminar
await api.stopScanStream();
await sub.cancel();
```

## Pontos cruciais
- É necessário rodar em Windows 10/11 com Bluetooth habilitado.
- O stream usa `BluetoothLEAdvertisementWatcher` (C++/WinRT). Durante o scan, mais anúncios implicam mais eventos; trate debounce/agrupamento no app se desejar.
- O endereço `id` já vem em formato MAC-like (hex com `:`); alguns dispositivos podem omitir `name` nos anúncios.
- Em caso de erro nativo, o stream pode emitir `error` via EventSink; trate com `listen(onError: ...)`.
- Ao distribuir o app, compile em Release e garanta que o runtime do Flutter esteja presente.

## Roadmap
- Conexão GATT, serviços/características, leitura/escrita e notificações
- Filtros por serviço (UUID) no scan
- Pareamento/consentimento quando necessário



