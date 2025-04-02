# Projeto Semáforo Inteligente para Pedestres

Desenvolvido durante o curso de capacitação Embarcatech, este projeto utiliza a placa BitDogLab com Raspberry Pi Pico W para criar um semáforo interativo para pedestres.

Funcionalidades:

✅ Matriz de LEDs WS2812B – Exibe ícones animados indicando "pare" (vermelho) e "siga" (verde).

✅ LEDs individuais – Sinalização clara com LEDs vermelho e verde.

✅ Display OLED – Mostra mensagens como "AGUARDE" ou "PODE ATRAVESSAR".

✅ Controle por botões físicos e Wi-Fi – O pedestre pode solicitar a abertura do sinal tanto por um botão físico quanto via página web.

✅ Servidor HTTP embarcado – Permite acionamento remoto via Wi-Fi (CYW43).

Tecnologias utilizadas:

- Raspberry Pi Pico W (com chip Wi-Fi CYW43439)

- Protocolo PIO para controle eficiente da matriz de LEDs

- Protocolo I2C para comunicação com o display OLED

- LWIP (TCP/IP stack) para servidor web local

Ideal para estudos em sistemas embarcados, IoT e automação urbana, este projeto combina hardware e software para criar uma solução prática e educativa. 🚦💡

(Desenvolvido como parte do curso Embarcatech - BitDogLab)
