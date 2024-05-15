create table if not exists users (
    user_id serial primary key,
    login text,
    email text,
    img_link text,
    city text,
    cash bigint
);