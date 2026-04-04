#!/bin/sh
set -eu

SYSTEMS_CFG="${ROOK_UI_ES_SYSTEMS_CFG:-/etc/emulationstation/es_systems.cfg}"
ROMS_PATH="${ROOK_UI_ES_ROMS_PATH:-/usr/share/rook-console-ui/emulationstation/roms}"
THEME_DIR="${ROOK_UI_ES_THEME_DIR:-/etc/emulationstation/themes/carbon-2021/rook}"
THEME_LOGO="${ROOK_UI_ES_THEME_LOGO:-/usr/share/rook-console-ui/emulationstation/logo/rook-system-logo.svg}"
TMP_FILE="$(mktemp)"
THEME_FILE="${THEME_DIR}/theme.xml"

cleanup() {
  rm -f "$TMP_FILE"
}

trap cleanup EXIT

ensure_theme_logo() {
  if [ -f "${THEME_FILE}" ] &&
     ! grep -q 'Managed by rook-console-integration' "${THEME_FILE}" &&
     ! grep -q 'rook_logo_v1-0-0_name_bw.svg' "${THEME_FILE}" &&
     ! grep -q '/usr/share/rook-console-ui/emulationstation/logo/rook-system-logo.svg' "${THEME_FILE}"; then
    return
  fi

  mkdir -p "${THEME_DIR}"
  cat > "${THEME_FILE}" <<EOF
<!-- Managed by rook-console-integration -->
<theme>
  <formatVersion>3</formatVersion>
  <view name="system, basic, detailed, grid, video">
    <image name="logo">
      <path>${THEME_LOGO}</path>
      <origin>0.5 0.5</origin>
      <pos>0.5 0.12</pos>
      <maxSize>0.7 0.18</maxSize>
    </image>
  </view>
</theme>
EOF
  chmod 0644 "${THEME_FILE}"
}

ensure_systems_cfg_mode() {
  if [ -f "$SYSTEMS_CFG" ]; then
    chmod 0644 "$SYSTEMS_CFG"
  fi
}

SYSTEM_BLOCK=$(cat <<EOF
  <system>
    <name>rook</name>
    <fullname>RooK</fullname>
    <path>${ROMS_PATH}</path>
    <extension>.sh .SH</extension>
    <command>/bin/sh %ROM%</command>
    <platform>custom</platform>
    <theme>rook</theme>
  </system>
EOF
)

if [ ! -f "$SYSTEMS_CFG" ]; then
  ensure_theme_logo
  mkdir -p "$(dirname "$SYSTEMS_CFG")"
  cat > "$SYSTEMS_CFG" <<EOF
<?xml version="1.0"?>
<systemList>
${SYSTEM_BLOCK}
</systemList>
EOF
  ensure_systems_cfg_mode
  exit 0
fi

if grep -qi '<name>[[:space:]]*rook[[:space:]]*</name>' "$SYSTEMS_CFG" || \
   grep -qi '<fullname>[[:space:]]*RooK[[:space:]]*</fullname>' "$SYSTEMS_CFG"; then
  ensure_theme_logo
  ensure_systems_cfg_mode
  exit 0
fi

ensure_theme_logo

awk -v system_block="$SYSTEM_BLOCK" '
  BEGIN { inserted = 0 }
  /<\/systemList>/ && inserted == 0 {
    print system_block
    inserted = 1
  }
  { print }
  END {
    if (inserted == 0) {
      print "<systemList>"
      print system_block
      print "</systemList>"
    }
  }
' "$SYSTEMS_CFG" > "$TMP_FILE"

mv "$TMP_FILE" "$SYSTEMS_CFG"
ensure_systems_cfg_mode
