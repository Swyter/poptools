//------------------------------------------------
//--- 010 Editor v13.0 Binary Template
//
//      File: Prince of Persia Trilogy - Reverse-engineered CRC32-checksumming functions
//   Authors: Swyter
//   Version: 2023.01.11
//  Category: Game
//------------------------------------------------
uint i = 0, j = 0;
char target_file_path[] = GetFileName(); char script_path[] = FileNameGetPath(GetScriptFileName()); SetWorkingDirectory(script_path);

string convert_win_to_wsl_path(char path[])
{
    string path_wsl = "";
    
    /* swy: convert the Windows paths to the format the Windows subsystem for Linux likes. i.e. \ to / and Y: to /mnt/y/ */
    for (i = 0; path[i] != 0; i++)
    {
        if (path[i] == '\\')
            path[i] = '/';
    
        if (path[i + 1] == ':')
            path[i] = ToLower(path[i]);
    
        if (path[i] == ':')
            continue;
    
        path_wsl+= path[i]; j++;
    }

    return Str("/mnt/" + path_wsl);
}

string path_target_wsl = convert_win_to_wsl_path(target_file_path);
string path_script_wsl = convert_win_to_wsl_path(script_path);


uint64 compute_crc_for_block_delsum(uint offset, uint size)
{
    // e.g. delsum check -m 'crc width=32 poly=0x4c11db7 init=0xffffffff xorout=0x0 refin=true refout=true out_endian=big' Profile.DAT -S 0x120 -E 0x19d # swy: from 0x120 to ((0x120 + 0x7e) - 1) = (0x19e - 1) = 0x19d
    uint32 start_offset = offset, end_offset = (offset + size) - 1; /* swy: convert it to delsum's weird byte range format */

    string param = Str("-- cd '%s' && pwd && echo `./delsum.elf check -m 'crc width=32 poly=0x4c11db7 init=0xffffffff xorout=0x0 refin=true refout=true out_endian=big' '%s' -S %#x -E %#x` > delsum_out.txt", path_script_wsl, path_target_wsl, start_offset, end_offset);
    
    int ret; Exec("wsl", param, /* swy: wait */ true, ret); /* swy: returning 0 is good, 127 is bad */
    
    if (ret != 0)
        return 0;
    
    int idx_act_file = GetFileNum();
    int idx_out_file = FileOpen(Str(script_path + "/delsum_out.txt"));

    if (idx_out_file == -1)
        return 0;
    
    string crc_str = ReadString(0); FileClose(); // FileDelete(Str(script_path + "/delsum_out.txt"));

    FileSelect(idx_act_file); /* swy: switch the context back to the actual profile.dat with the template we're working on */

    uint32 crc; SScanf(crc_str, "%x", crc);
    return crc;
}

/* -- */

/* swy: function starts at 0x0420080 on POP2.exe; you can also search
        by the 0x4c11db7 polynomial constant: 'crc width=32 poly=0x4c11db7 init=0xffffffff xorout=0x0 refin=true refout=true out_endian=big' */
void generate_checksum_lookup(uint dest_buf[]) 
{
    uint val_a = 0, j, val = 0, idx = 0;
    
    for (idx = 0; idx < 256; idx += 1)
    {
        if ((idx >> 0 & 1) != 0)  val_a  = 128;
        if ((idx >> 1 & 1) != 0)  val_a |=  64;
        if ((idx >> 2 & 1) != 0)  val_a |=  32;
        if ((idx >> 3 & 1) != 0)  val_a |=  16;
        if ((idx >> 4 & 1) != 0)  val_a |=   8;
        if ((idx >> 5 & 1) != 0)  val_a |=   4;
        if ((idx >> 6 & 1) != 0)  val_a |=   2;
        if ((idx >> 6 & 2) != 0)  val_a |=   1;
        
        val_a = -(uint)((val_a & 0x80)       != 0) & 0x4c11db7 ^ val_a * 0x2000000;
        val_a = -(uint)((val_a & 0x80000000) != 0) & 0x4c11db7 ^ val_a * 2;
        val_a = -(uint)((val_a & 0x80000000) != 0) & 0x4c11db7 ^ val_a * 2;
        val_a = -(uint)((val_a & 0x80000000) != 0) & 0x4c11db7 ^ val_a * 2;
        val_a = -(uint)((val_a & 0x80000000) != 0) & 0x4c11db7 ^ val_a * 2;
        val_a = -(uint)((val_a & 0x80000000) != 0) & 0x4c11db7 ^ val_a * 2;
        val_a = -(uint)((val_a & 0x80000000) != 0) & 0x4c11db7 ^ val_a * 2;
        val_a = -(uint)((val_a & 0x80000000) != 0) & 0x4c11db7 ^ val_a * 2;
        
        dest_buf[idx] = val_a; Printf("%3i, a - %#010x, ", idx, dest_buf[idx]);
        
        for (j = 31, val = 0; j < 0x80000000; j -= 1)
        {
            if ((val_a & 1) != 0)
                val |= 1 << ((byte) j & 31);
            val_a >>= 1;
        }
        
        dest_buf[idx] = val; Printf("final - %08x\n", dest_buf[idx]);
    }
    return;
}

/* swy: the game fills out the CRC look-up table buffer at 0x93F2F8 on POP2.exe during startup,
        instead of including the constants statically in the binary */
uint precomputed_table[256]; generate_checksum_lookup(precomputed_table);

/* swy: uses the precomputed look-up table to generate a CRC32 over a buffer range, see function 0x0420040 on POP2.exe */
uint compute_crc_for_block(uint crc_lut_buf[], uint offset, uint size)
{
    uint checksum = 0xffffffff; local uint i = 0;

    if (size > 0)
        for (i = 0; i < size; i++)
            checksum = checksum >> 8 ^ crc_lut_buf[checksum & 0xff ^ ReadUByte(offset + i)];

    return checksum;
}
