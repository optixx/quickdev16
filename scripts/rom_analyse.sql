-- Global
select count(*) from roms;
select distinct rom_name from roms;
select count(*) from roms where rom_sram = 0;

-- Hirom x Sram
select count(*) from roms where rom_hirom= 1;
select count(*),rom_internalsize from roms where rom_hirom= 1 group by rom_internalsize;
select count(*) as count ,rom_internalsize,rom_sram from roms where rom_hirom= 1 group by rom_internalsize,rom_sram having rom_sram > 0;

-- Lorom  x Sram
select count(*) from roms where rom_hirom= 0;
select count(*),rom_internalsize from roms where rom_hirom= 0 group by rom_internalsize;
select count(*) as count ,rom_internalsize,rom_sram from roms where rom_hirom= 0 group by rom_internalsize,rom_sram having rom_sram > 0;

-- Sram
select count(*) as count ,rom_sram from roms group by rom_sram having count > 1;

-- Rom
select count(*) as count ,rom_size from roms group by rom_size having count > 1;
select count(*) as count ,rom_size from roms group by rom_size having count > 10;

select count(*) as count ,rom_internalsize from roms group by rom_internalsize having count > 1;
select count(distinct rom_name) as count ,rom_internalsize from roms group by rom_internalsize having count > 1;
select count(*) as count ,rom_size,rom_internalsize from roms group by rom_size having count > 10;


-- Type

--  00   ROM
--  01   ROM/RAM
--  02   ROM/SRAM
--  03   ROM/DSP1
--  04   ROM/DSP1/RAM
--  05   ROM/DSP1/SRAM
--  06   FX
select count(*) as count ,rom_type from roms group by rom_type having count > 1;
select count(*) as count ,rom_type from roms group by rom_type having count > 5;

select count(*) as count  from roms where rom_type > 2;
select count(*) as count  from roms where rom_type <= 2;



--
-- cleanup
--
--delete from roms where rom_vendor = 'Demo or Beta ROM?';
--delete from roms where rom_internalsize > 128;
--delete from roms where rom_name like "%.....%";
--delete from roms where rom_name like "%\%";
--delete from roms where rom_vendor = 'Unknown';

