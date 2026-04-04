#!/bin/sh
set -eu

SYSTEMS_CFG="${ROOK_UI_ES_SYSTEMS_CFG:-/etc/emulationstation/es_systems.cfg}"
ROMS_PATH="${ROOK_UI_ES_ROMS_PATH:-/usr/share/rook-console-ui/emulationstation/roms}"
TMP_FILE="$(mktemp)"

cleanup() {
  rm -f "$TMP_FILE"
}

trap cleanup EXIT

cleanup_legacy_theme_snippets() {
  for theme_file in \
    /etc/emulationstation/themes/carbon-2021/rook/theme.xml \
    /etc/emulationstation/themes/carbon/rook/theme.xml
  do
    if [ ! -f "${theme_file}" ]; then
      continue
    fi

    if grep -q 'Managed by rook-console-integration' "${theme_file}" || \
       grep -q 'rook_logo_v1-0-0_name_bw.svg' "${theme_file}" || \
       grep -q '/usr/share/rook-console-ui/emulationstation/logo/rook-system-logo.svg' "${theme_file}"; then
      rm -f "${theme_file}"
      rmdir --ignore-fail-on-non-empty "$(dirname "${theme_file}")" 2>/dev/null || true
    fi
  done
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
  cleanup_legacy_theme_snippets
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
  cleanup_legacy_theme_snippets
  ensure_systems_cfg_mode
  exit 0
fi

cleanup_legacy_theme_snippets

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
