# ESP-IDF en VS Code (WSL Debian)

Guia reproducible para instalar y dejar funcionando la extension ESP-IDF en VS Code dentro de WSL Debian.

## 1. Verificar herramientas base

Ejecuta:

```bash
python3 --version
which python3
which uv
which conda
```

Notas:
- Si `uv` existe, usar `uv` (recomendado por velocidad).
- Si no existe, puedes usar conda o el entorno que cree automaticamente el instalador de ESP-IDF.

## 2. Crear entorno virtual con UV (opcional pero recomendado)

```bash
mkdir -p ~/.espressif
uv venv ~/.espressif/python_env/idf_uv_venv --python python3
```

Activacion manual si lo necesitas:

```bash
source ~/.espressif/python_env/idf_uv_venv/bin/activate
```

## 3. Instalar la extension de VS Code

Extension ID:

```text
espressif.esp-idf-extension
```

Desde terminal de VS Code:

```bash
code --install-extension espressif.esp-idf-extension
```

## 4. Instalar prerequisitos del sistema (Debian/Ubuntu)

```bash
sudo apt-get update
sudo apt-get install -y flex bison gperf ccache dfu-util cmake ninja-build libffi-dev libssl-dev
```

## 5. Instalar ESP-IDF con Installation Manager (EIM)

La extension instala el binario `eim` en:

```text
~/.espressif/eim_gui/eim
```

Instalacion no interactiva (v5.4):

```bash
~/.espressif/eim_gui/eim install \
	--non-interactive true \
	--idf-versions v5.4 \
	--path ~/esp \
	--skip-prerequisites-check true \
	--python-env-folder-name python \
	--target all 2>&1 | tee /tmp/eim_install.log
```

Monitoreo:

```bash
tail -f /tmp/eim_install.log
```

## 6. Comando correcto en VS Code (extension v2.x)

En esta version, el comando de paleta es:

```text
ESP-IDF: Open ESP-IDF Installation Manager
```

No siempre aparece el comando antiguo de configuracion. Usa este y luego importa/selecciona la instalacion creada por EIM.

## 7. Rutas que debes verificar al final

- ESP-IDF path: `~/esp/v5.4/esp-idf`
- Tools path: `~/.espressif/tools`
- Python env de EIM: `~/.espressif/python`
- Tu UV venv opcional: `~/.espressif/python_env/idf_uv_venv`

## 8. Verificacion rapida

```bash
~/.espressif/eim_gui/eim list
ls ~/esp/v5.4/esp-idf
```

Si todo esta bien, `eim list` muestra la version instalada y el directorio `~/esp/v5.4/esp-idf` existe.

## 9. Troubleshooting comun

- Error de prerequisitos faltantes aunque ya estan instalados:
	usa `--skip-prerequisites-check true`.

- No aparece el comando que esperabas en la paleta:
	en v2.x usa `ESP-IDF: Open ESP-IDF Installation Manager`.

- Instalacion tarda mucho:
	es normal (descarga toolchains + submodulos). Revisa progreso con `tail -f /tmp/eim_install.log`.

## 10. Script listo para reutilizar (Debian/WSL)

Guarda y ejecuta esto si quieres repetir todo de forma automatica:

```bash
#!/usr/bin/env bash
set -euo pipefail

sudo apt-get update
sudo apt-get install -y flex bison gperf ccache dfu-util cmake ninja-build libffi-dev libssl-dev

mkdir -p ~/.espressif

if command -v uv >/dev/null 2>&1; then
	uv venv ~/.espressif/python_env/idf_uv_venv --python python3
fi

if ! command -v code >/dev/null 2>&1; then
	echo "VS Code CLI ('code') no esta en PATH. Abre VS Code e instala la extension manualmente."
else
	code --install-extension espressif.esp-idf-extension || true
fi

if [ -x ~/.espressif/eim_gui/eim ]; then
	~/.espressif/eim_gui/eim install \
		--non-interactive true \
		--idf-versions v5.4 \
		--path ~/esp \
		--skip-prerequisites-check true \
		--python-env-folder-name python \
		--target all 2>&1 | tee /tmp/eim_install.log
else
	echo "No se encontro ~/.espressif/eim_gui/eim."
	echo "Abre VS Code y ejecuta: ESP-IDF: Open ESP-IDF Installation Manager"
fi

echo "Terminado. Revisa: /tmp/eim_install.log"
```
