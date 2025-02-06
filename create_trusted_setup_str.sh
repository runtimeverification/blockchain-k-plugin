#!bin/sh

IN=$1
OUT=$2

echo -n "const char *trusted_setup_str = R\"(" > "$OUT"

first_line=1
while IFS='' read -r line; do
  if [ "$first_line" = "0" ]; then
    echo >> "$OUT"
  else
    first_line=0
  fi
  echo -n "$line" >> "$OUT"
done < "$IN"

echo -n ")\";" >> "$OUT"
