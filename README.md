<p align="center">
  <img src="cadence.svg" alt="Cadence" width="128" height="128">
</p>

<h1 align="center">Cadence</h1>

<p align="center">
  Emulador de <b>Amstrad CPC</b> escrito en C++ y Qt 6, con debugger y ensamblador integrados.
</p>

---

## Características

### Sistemas emulados

- **Amstrad CPC 464**, **CPC 664** y **CPC 6128** — seleccionables desde el menú *System*.
- **Variantes de CRTC** 0 a 4 — configurables en *Preferences* (cada chip tiene su comportamiento; relevante para demos y juegos que dependen del modelo).
- **Expansión opcional de 512 KB** de RAM (estilo DK'tronics).

### Núcleo del emulador

- **CPU Z80** completa: instrucciones base, prefijos CB (bit/rotaciones), DD/FD (IX/IY), DD/FD CB (IX/IY indexado a bit), ED (E/S, bloques) e interrupciones.
- Registros principales y alternativos (AF, BC, DE, HL y sus primados; IX, IY, I, R, SP, PC) con flags Z80 correctos.
- **Gate Array**, **CRTC**, **PSG AY-3-8912**, **PPI 8255** y **FDC 765** emulados como módulos independientes.

### Vídeo

- Renderizado vía **OpenGL** con PBO para volcado eficiente desde el hilo de emulación.
- **Modo monitor verde** (monocromo).
- **Persistencia de fósforo** configurable de 0 a 5 frames (`Ctrl+0` … `Ctrl+5`).
- **Filtrado/smoothing** activable (`Ctrl+F11`).
- **Pantalla completa** (`F11`).
- Marco gráfico de monitor CRT.

### Audio

- Salida vía **PortAudio**.
- **Audio del PSG** activable/desactivable.
- **Efectos de sonido** del hardware (motor del disco, paso del cabezal).
- **Audio de la cinta** opcional.

### Soportes / medios

| Slot | Formatos | Operaciones |
|---|---|---|
| **Disco A: y B:** | `.DSK` estándar y `.DSK` **EXTENDED** | Insertar, extraer, write-protect, crear disco vacío |
| **Cinta** | `.CDT` (ZXTape!), `.WAV` | Insertar, extraer, rebobinar; indicador de progreso |
| **Cartucho** | `.CPR` (Plus / GX4000) | Insertar, extraer, cartucho en blanco; **auto-run** al insertar |
| **ROM Box** | 16 slots de ROMs | Diálogo dedicado |

- **ROMs por defecto embebidas** en el binario: BIOS y BASIC del 464, 664 y 6128, AMSDOS 6128.
- **Drag & drop** de archivos sobre la ventana principal.

### Entrada

- **Teclado** mapeado posicionalmente al teclado del CPC.
- Opción **Right-Shift como backslash** (útil para layouts no-UK).
- **Emulación de joystick por teclado** (`F10`): cursores + `Espacio`/`Ctrl`.

### Debugger (`F5`)

- **Panel de registros Z80** editable en vivo: AF/BC/DE/HL y primados, IX, IY, SP, PC, I, R, flags.
- **Panel del CRTC** y **panel del Gate Array** (editables).
- **Panel de la pila** con navegación.
- **Desensamblador** con cursor y soporte para anclas manuales que reinterpretan bytes.
- **Ejecución paso a paso**: Run (`F5`), Step In (`F7`), Step Over (`F8`), Step Out (`F6`), **Run to cursor** (`F4`).
- **Breakpoints condicionales**: expresiones evaluadas al alcanzar la dirección, con registros, comparaciones y operadores `<<`, `>>`, `<`, `<=`, `>`, `>=`, `&&`, `||`. Ejemplo: `HL >= &C000 && HL < &D000`.
- **Inspector de memoria** con múltiples fuentes:
  - Vista de CPU (mapping actual)
  - Bancos de RAM (`#C0`..`#C7`)
  - Lower ROM
  - Cualquiera de los 16 slots de Upper ROM
  - Cartucho
- **Buscar bytes** (`Ctrl+F`) y **buscar siguiente** (`Ctrl+G`).
- **Cargar / guardar binarios** desde / a memoria.
- **Entrada manual de bytes**.
- **Go to address** (`F3`).
- **Contador de NOPS / T-states** con reset (`F12`).
- **Inspector de memoria de vídeo** en ventana separada.

### Ensamblador integrado (`F6`)

- **Editor multipestaña** con sesión persistente.
- **Resaltado de sintaxis** Z80.
- **Output con errores enlazables**: clic en el mensaje navega a la línea fuente.
- **Directivas soportadas**:
  - Origen y *program counter*
  - Asignación de símbolos
  - Emisión de datos
  - I/O de archivos
  - **Targets de salida múltiples**: memoria directa (con control de Lower ROM / Upper ROM / banco RAM), archivos AMSDOS en disco, **sectores DSK específicos** por pista/sector
  - Ensamblaje condicional
  - Bucles
  - Macros
  - Diagnósticos y control
- **Ejecución tras ensamblar** con breakpoint opcional en la dirección de inicio.

### Persistencia y configuración

- **Settings** guardados entre sesiones:
  - Preferencias de vídeo (verde, smoothing, persistencia, pantalla completa)
  - Audio (PSG, SFX, cinta)
  - CRTC, expansión 512K, modelo de CPC
  - Estado de breakpoints (lista + condiciones)
  - Joystick y mapeo de teclas
  - Paths recientes de disco / cinta / cartucho / ROMs / ensamblador
- **ROMs personalizadas** sobreescribibles por slot.

### Otras utilidades

- **Pausa** de la emulación.
- **Reset** (`F12`) sin perder los medios insertados.
- **Unlock speed** (`F9`): elimina el tope de 50 Hz.
- **Diálogos de ayuda** para sintaxis de breakpoints y directivas del ensamblador.
- **Quick Start** integrado para usuarios nuevos.

---

## Compilar desde el código fuente

### Dependencias

- **Qt 6** (Core, Gui, Widgets, OpenGL, OpenGLWidgets, Svg)
- **PortAudio** (paquete `portaudio-2.0` vía pkg-config)
- Compilador con soporte de **C++17**

### Linux (Debian / Ubuntu / Mint)

```bash
sudo apt install qt6-base-dev qt6-svg-dev libqt6opengl6-dev \
                 libportaudio2 portaudio19-dev build-essential
qmake6 Cadence.pro
make -j"$(nproc)"
./cadence
```

### macOS

```bash
brew install qt portaudio
qmake Cadence.pro     # o la ruta absoluta a qmake de tu instalación de Qt
make -j"$(sysctl -n hw.ncpu)"
open cadence.app
```

## Generar un instalador

El repositorio incluye dos scripts que producen un instalador autocontenido (Qt y PortAudio bundleados):

| Plataforma | Script | Salida |
|---|---|---|
| Linux | `./create_installer_linux.sh` | `build/Cadence-<ver>-x86_64.AppImage` |
| macOS | `./create_installer.sh` | `build/Cadence-Installer.dmg` |

El script de Linux descarga automáticamente `linuxdeploy` y el plugin de Qt la primera vez que se ejecuta.

---

## Cómo usarlo

| Tecla | Acción |
|---|---|
| `F1` | Insertar cinta |
| `Shift+F1` | Rebobinar cinta |
| `Ctrl+F1` | Extraer cinta |
| `F2` | Insertar disco en unidad A |
| `Ctrl+F2` | Extraer disco de A |
| `F3` | Insertar cartucho |
| `Ctrl+F3` | Extraer cartucho |
| `F5` | Abrir el debugger |
| `F6` | Abrir el ensamblador |
| `F9` | Activar/desactivar *unlock speed* |
| `F10` | Activar/desactivar emulación de joystick |
| `F11` | Pantalla completa |
| `Ctrl+F11` | Smoothing |
| `Shift+F11` | Monitor verde |
| `F12` | Reset |
| `Ctrl+0`…`Ctrl+5` | Persistencia de fósforo |

Una vez insertado un medio:

- **Cinta**: `RUN"` (o `RUN"name` para un fichero concreto).
- **Disco**: `RUN"DISC` para el cargador por defecto, o `CAT` para listar.
- **Cartucho**: arranca automáticamente al insertarlo.

---

## Autor

© Abalore, 2026.
