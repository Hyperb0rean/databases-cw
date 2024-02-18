create table crops (
    crop_id serial primary key, 
    name text not null,
    humidity real check (humidity >= 0 and humidity <= 100),
    brightness real check (brightness > 0),
    is_legal boolean not null,
    brain_damage real check (brain_damage > -200 and brain_damage < 200)
);

create table estate (
    estate_id serial primary key, 
    name text not null,
    area real not null check (area > 100),
    county text not null,
    rent real not null check (rent > 0)
);

create table plantation (
    plantation_id serial primary key,
    capacity integer not null,
    revenue real not null,
    estate_id integer references estate(estate_id)
);

create table crops_plantations (
    crop_id integer references crops(crop_id),
    plantation_id integer references plantation(plantation_id),
    counter integer check (counter >= 0),
    primary key(crop_id, plantation_id)
);

create table worker (
    worker_id serial primary key,
    plantation_id integer references plantation(plantation_id),
    name text not null,
    salary real check (salary >= 0),
    profession text not null
);

create table worker_family_member (
    worker_fm_id serial primary key,
    worker_id integer references worker(worker_id),
    name text not null,
    relationship text not null,
    adress text not null
);

create table landlord (
    landlord_id serial primary key,
    estate_id integer references estate(estate_id),
    name text not null,
    capital real check (capital > 0)
);

create table evidence_info (
    evidence_info_id serial primary key,
    landlord_id integer references landlord(landlord_id),
    description text,
    worth real check (worth >= 20 AND worth <= 100)
);

create table manager (
    manager_id serial primary key,
    plantation_id integer references plantation(plantation_id),
    name text not null,
    salary real check (salary >=100)
);

create table client (
    client_id serial primary key,
    manager_id integer references manager(manager_id),
    name text not null,
    debt real check (debt < 10000),
    is_highly_addicted boolean,
    brain_resource real
);

create table client_family_member(
    client_fm serial primary key,
    client_id integer references client(client_id),
    adress text not null,
    email text unique check (email ~ '^[a-zA-Z0-9.!#$%&''*+/=?^_`{|}~-]+@[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?(?:\.[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?)*$')
);