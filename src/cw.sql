create or replace function  cell(id_client integer, id_crop integer, amount integer)
returns void as $$
    begin
        update crops_plantations set counter = counter - amount where crop_id = id_crop and counter >= amount;
        update client set brain_resource = brain_resource - (select brain_damage from crops where crop_id = id_crop) where client_id = id_client;
    end;
$$ language plpgsql;




create or replace function killer_f()
returns trigger as $$
    begin
        if (NEW.brain_resource <= 0)
        then begin
                delete from client_family_member where client_id = NEW.client_id;
                delete from client where client_id = NEW.client_id;
                return null;
            end;
        else return NEW;
    end;
$$ language plpgsql;


create trigger killer_t
before insert or update on client
for each row execute function killer_f();



create or replace function plant_capacity_checker_f()
returns trigger as $$
    begin
        if (select sum(counter) from crops_plantations where plantation_id = NEW.plantation_id) 
           + 
           (NEW.counter - OLD.counter)
           > (select capacity from  plantation where plantation_id = NEW.plantation_id)
       then 
            begin
                raise notice 'please add new plantation';
                return null;
            end; 
        else return NEW; 
       end if;
    end;
$$ language plpgsql;


create trigger plant_capacity_checker_t
before insert or update on crops_plantations
for each row execute function plant_capacity_checker_f();


create or replace function enum_crops_on_plantation(name text)
returns setof text as $$
    begin
        select name from crops c
        join crops_plantations cp on c.crops_id = cp.crops_id
        join plantation p on cp.plantation_td = p.plantation_id
        where p.name = name;
    end;
$$ language plpgsql;


create or replace function get_friends_email(name text)
returns setof text as $$
    begin
        select email from client_family_member
        where client_id = (select client_id from client where client.name = name);
    end;
$$ language plpgsql;

create index crop_id_i on crops using hash(crop_id);
create index client_id_i on client using hash(client_id);
create index brain_damage_i on crops using btree(brain_damage);

