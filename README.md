# embarcado

## Dependências

- VSCode
- Extenção Platformio instalada no VSCode

## Configuração

Na página do `PIO Home`, selecione o menu `Platforms` e instale a plataforma `Espressif 32`.

Abra o diretório raiz deste repositório no vscode com a extenção Platformio já instalada. A extenção deve reconhecer o projeto e fazer as devidas configurações.

## Build

A aba da extenção do plataformio tem uma área de tasks relacionadas a ações que podem ser aplicadas durante o desenvolvimento do dispositivo. Para realizar a build do projeto, vá em `Project Tasks -> nodemcu-32s -> General -> Build`.

Junto a opção de build, há outras tarefas como `Upload`, `Monitor` e `Clean`.

### Patches

A versão estável atual da esp-idf tem um problema de importação em um dos arquivos do wifi. 
Vá para o arquivo `src/wifi.h`, navegue até a função com o nome `esp_netif_create_default_wifi_ap();`, e vá para a referência com `ctrl+click`. No novo arquivo aberto, navegue até o topo, e adicione o include `#include "esp_wifi_types.h"`.
