#!/bin/bash
set -e

psql -v ON_ERROR_STOP=1 "postgresql://localhost:0239/postgres?user=postgres&password=qwerty" <<-EOSQL
    insert into cities (id, name) values
    (1, 'Moscow'),
    (2, 'Saint-Petersburg');
EOSQL