create table if not exists users (
    user_id serial primary key,
    login text,
    hash_password text,
    email text,
    img_link text,
    city text,
    cash bigint
);

create table if not exists tokens (
    id serial primary key,
    user_id bigint,
    hash_refresh_token text,
    hash_access_token text
);

create table if not exists items (
    id serial primary key,
    name text,
    description text,
    cost_usd bigint,
    media text[],
    preview text,
    organization text
);

create table if not exists organizations (
    id serial primary key,
    name text not null,
    city_id bigint
);

create table if not exists cities (
    id bigint primary key,
    name text
);

create table if not exists items_count (
    item_id bigint,
    organization_id bigint,
    item_count bigint not null,
    foreign key (item_id) references items(id),
    foreign key (organization_id) references organizations(id)
);

create table if not exists busket (
    id serial primary key,
    user_id bigint
);

create table if not exists items_busket (
    item_id bigint,
    busket_id bigint,
    item_count bigint
);