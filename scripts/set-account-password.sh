#!/bin/bash
# Imposta password account (tabella user, non toon).
# Uso: ./scripts/set-account-password.sh <user_id> <password> [email]
set -euo pipefail

USER_ID="${1:?user_id}"
PASSWORD="${2:?password}"
EMAIL="${3:-}"
CONTAINER="${NEBBIE_CONTAINER:-nebbieserver}"

TMP_SQL="$(mktemp)"
trap 'rm -f "$TMP_SQL"' EXIT

python3 - "$USER_ID" "$PASSWORD" "$EMAIL" "$TMP_SQL" <<'PY'
import crypt, sys
user_id, password, email, out_path = sys.argv[1:5]
h = crypt.crypt(password, crypt.mksalt(crypt.METHOD_MD5))
where = f"id={int(user_id)}"
if email:
    where += f" AND email='{email.replace(chr(39), chr(39)*2)}'"
sql = f"UPDATE user SET password='{h}' WHERE {where};\n"
sql += f"SELECT id, email, password FROM user WHERE id={int(user_id)};\n"
open(out_path, "w").write(sql)
PY

docker exec -i "$CONTAINER" mysql -h 127.0.0.1 -uroot -psecret nebbie <"$TMP_SQL"

docker exec "$CONTAINER" python3 -c "
import crypt, subprocess
p = subprocess.check_output([
    'mysql', '-h', '127.0.0.1', '-uroot', '-psecret', '-N', 'nebbie',
    '-e', 'SELECT password FROM user WHERE id=${USER_ID}'
], text=True).strip()
print('Verifica password: OK' if crypt.crypt('${PASSWORD}', p) == p else 'Verifica password: FAIL')
" 2>/dev/null
