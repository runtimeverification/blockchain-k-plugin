#!bin/sh

IN=$1
OUT=$2

printf '%s' 'const char *trusted_setup_str = R"(' > "$OUT"

first_line=1
while IFS='' read -r line; do
  if [ "$first_line" = "0" ]; then
    printf "\n" >> "$OUT"
  else
    first_line=0
  fi
  printf "%s" "$line" >> "$OUT"
done < "$IN"

printf "%s" ')";' >> "$OUT"
