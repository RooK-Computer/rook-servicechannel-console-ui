#!/bin/sh
set -eu

SYSTEMS_CFG="${ROOK_UI_ES_SYSTEMS_CFG:-/etc/emulationstation/es_systems.cfg}"
ROMS_PATH="${ROOK_UI_ES_ROMS_PATH:-/usr/share/rook-console-ui/emulationstation/roms}"
THEME_DIR="${ROOK_UI_ES_THEME_DIR:-/etc/emulationstation/themes/carbon/rook}"
THEME_LOGO="${ROOK_UI_ES_THEME_LOGO:-/usr/share/rook-console-ui/resources/rook_logo_v1-0-0_name_bw.svg}"
TMP_FILE="$(mktemp)"

cleanup() {
  rm -f "$TMP_FILE"
}

trap cleanup EXIT

ensure_theme_logo() {
  if [ -f "${THEME_DIR}/theme.xml" ]; then
    return
  fi

  mkdir -p "${THEME_DIR}"
  cat > "${THEME_DIR}/theme.xml" <<EOF
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
  exit 0
fi

if grep -qi '<name>[[:space:]]*rook[[:space:]]*</name>' "$SYSTEMS_CFG" || \
   grep -qi '<fullname>[[:space:]]*RooK[[:space:]]*</fullname>' "$SYSTEMS_CFG"; then
  ensure_theme_logo
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
