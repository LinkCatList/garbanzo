create table if not exists users (
    user_id serial primary key,
    login text,
    email text,
    img_link text,
    city text,
    cash bigint
);

create table if not exists favourites (
    user_id bigint,
    item_id bigint
);