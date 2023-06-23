#include "rpi.h"
#include "fat32.h"
#include "fat32-helpers.h"
#include "pi-sd.h"

// Print extra tracing info when this is enabled.  You can and should add your
// own.
static int trace_p = 1; 
static int init_p = 0;

fat32_boot_sec_t boot_sector;


fat32_fs_t fat32_mk(mbr_partition_ent_t *partition) {
  demand(!init_p, "the fat32 module is already in use\n");
  // TODO: Read the boot sector (of the partition) off the SD card.
  // unimplemented();
  // printk("the partition->lba_start is %d\n", partition->lba_start);
  pi_sd_read(&boot_sector, partition->lba_start, 1);

  // TODO: Verify the boot sector (also called the volume id, `fat32_volume_id_check`)
  // unimplemented();
  fat32_volume_id_print("check volume ID", &boot_sector);

  // TODO: Read the FS info sector (the sector immediately following the boot
  // sector) and check it (`fat32_fsinfo_check`, `fat32_fsinfo_print`)
  assert(boot_sector.info_sec_num == 1);
  // unimplemented();
  struct fsinfo *info = kmalloc(sizeof(struct fsinfo));
  // printk("the partition->lba_start + boot_sector.info_sec_num is %d\n", partition->lba_start + boot_sector.info_sec_num);
  pi_sd_read(info, partition->lba_start + boot_sector.info_sec_num, 1);
  fat32_fsinfo_print("fat32_fsinfo_print", info);
  fat32_fsinfo_check(info);
  
  // END OF PART 2
  // The rest of this is for Part 3:

  // TODO: calculate the fat32_fs_t metadata, which we'll need to return.
  unsigned lba_start = -1; // from the partition
  unsigned fat_begin_lba = -1; // the start LBA + the number of reserved sectors
  unsigned cluster_begin_lba = -1; // the beginning of the FAT, plus the combined length of all the FATs
  unsigned sec_per_cluster = -1; // from the boot sector
  unsigned root_first_cluster = -1; // from the boot sector
  unsigned n_entries = -1; // from the boot sector
  // unimplemented();

  lba_start = partition->lba_start;
  fat_begin_lba = lba_start + boot_sector.reserved_area_nsec; // * boot_sector.bytes_per_sec;
  cluster_begin_lba = fat_begin_lba + (boot_sector.nfats * boot_sector.nsec_per_fat); // * boot_sector.bytes_per_sec);
  sec_per_cluster = boot_sector.sec_per_cluster;
  root_first_cluster = boot_sector.first_cluster;
  n_entries = (boot_sector.nsec_per_fat * boot_sector.bytes_per_sec) >> 5;

  /*
   * TODO: Read in the entire fat (one copy: worth reading in the second and
   * comparing).
   *
   * The disk is divided into clusters. The number of sectors per
   * cluster is given in the boot sector byte 13. <sec_per_cluster>
   *
   * The File Allocation Table has one entry per cluster. This entry
   * uses 12, 16 or 28 bits for FAT12, FAT16 and FAT32.
   *
   * Store the FAT in a heap-allocated array.
   */
  uint32_t len = boot_sector.nfats * boot_sector.nsec_per_fat * boot_sector.bytes_per_sec;
  printk("the len is %d:\n", len);
  uint32_t *fat = kmalloc(len);

  // printk("the fat_begin_lba is %d\n", fat_begin_lba);
  // printk("the boot_sector.nfats * boot_sector.nsec_per_fat is %d\n", boot_sector.nfats * boot_sector.nsec_per_fat);
  pi_sd_read(fat, fat_begin_lba, boot_sector.nfats * boot_sector.nsec_per_fat);

  // unimplemented();


  // Create the FAT32 FS struct with all the metadata
  fat32_fs_t fs = (fat32_fs_t) {
    .lba_start = lba_start,
      .fat_begin_lba = fat_begin_lba,
      .cluster_begin_lba = cluster_begin_lba,
      .sectors_per_cluster = sec_per_cluster,
      .root_dir_first_cluster = root_first_cluster,
      .fat = fat,
      .n_entries = n_entries,
  };

  if (trace_p) {
    trace("begin lba = %x, int = %d\n", fs.fat_begin_lba, fs.fat_begin_lba);
    trace("cluster begin lba = %x, int = %d\n", fs.cluster_begin_lba, fs.cluster_begin_lba);
    trace("sectors per cluster = %d\n", fs.sectors_per_cluster);
    trace("root dir first cluster = %d\n", fs.root_dir_first_cluster);
  }

  init_p = 1;
  return fs;
}

// Given cluster_number, get lba.  Helper function.
static uint32_t cluster_to_lba(fat32_fs_t *f, uint32_t cluster_num) {
  assert(cluster_num >= 2);
  // TODO: calculate LBA from cluster number, cluster_begin_lba, and
  // sectors_per_cluster

  //cluster lba = cluster_begin + (cluster_number - 2) * sectors_per_cluster
  // unimplemented();
  unsigned lba = f->cluster_begin_lba + (cluster_num - 2) * f->sectors_per_cluster;
  if (trace_p) trace("cluster %d to lba: %d\n", cluster_num, lba);
  return lba;
}

pi_dirent_t fat32_get_root(fat32_fs_t *fs) {
  demand(init_p, "fat32 not initialized!");
  // TODO: return the information corresponding to the root directory (just
  // cluster_id, in this case)
  // unimplemented();


  return (pi_dirent_t) {
    .name = "",
      .raw_name = "",
      .cluster_id = 2, // fix this
      .is_dir_p = 1,
      .nbytes = 0,
  };
}

// Given the starting cluster index, get the length of the chain.  Helper
// function.
static uint32_t get_cluster_chain_length(fat32_fs_t *fs, uint32_t start_cluster) {
  // TODO: Walk the cluster chain in the FAT until you see a cluster where
  // `fat32_fat_entry_type(cluster) == LAST_CLUSTER`.  Count the number of
  // clusters.
  // unimplemented();
  uint32_t cnt = 1;
  while(1) {
    if(fat32_fat_entry_type(fs->fat[start_cluster]) == LAST_CLUSTER) {
      break;
    }
    start_cluster = fs->fat[start_cluster];
    ++cnt;
  }
  return cnt;
}

// Given the starting cluster index, read a cluster chain into a contiguous
// buffer.  Assume the provided buffer is large enough for the whole chain.
// Helper function.
static void read_cluster_chain(fat32_fs_t *fs, uint32_t start_cluster, fat32_dirent_t *data) {
  // TODO: Walk the cluster chain in the FAT until you see a cluster where
  // fat32_fat_entry_type(cluster) == LAST_CLUSTER.  For each cluster, copy it
  // to the buffer (`data`).  Be sure to offset your data pointer by the
  // appropriate amount each time.
  // unimplemented();
  int i = 0;
  while(1) {
    start_cluster = (start_cluster << 4) >> 4;
    // printk("the start_cluster is %d\n", start_cluster);
    pi_sd_read(data + fs->sectors_per_cluster * NBYTES_PER_SECTOR * i / sizeof(fat32_dirent_t), cluster_to_lba(fs, start_cluster), fs->sectors_per_cluster);
    if(fat32_fat_entry_type(fs->fat[start_cluster]) == LAST_CLUSTER) {
      break;
    }
    start_cluster = fs->fat[start_cluster];
    i++;
  }
}

// Converts a fat32 internal dirent into a generic one suitable for use outside
// this driver.
static pi_dirent_t dirent_convert(fat32_dirent_t *d) {
  pi_dirent_t e = {
    .cluster_id = fat32_cluster_id(d),
    .is_dir_p = d->attr == FAT32_DIR,
    .nbytes = d->file_nbytes,
  };
  // can compare this name
  memcpy(e.raw_name, d->filename, sizeof d->filename);
  // for printing.
  fat32_dirent_name(d,e.name,sizeof e.name);
  return e;
}

// Gets all the dirents of a directory which starts at cluster `cluster_start`.
// Return a heap-allocated array of dirents.
static fat32_dirent_t *get_dirents(fat32_fs_t *fs, uint32_t cluster_start, uint32_t *dir_n) {
  // TODO: figure out the length of the cluster chain (see
  // `get_cluster_chain_length`)
  // unimplemented();
  uint32_t length = get_cluster_chain_length(fs, cluster_start);
  printk("the chain_length is %d\n", length);

  // TODO: allocate a buffer large enough to hold the whole directory
  // unimplemented();
  fat32_dirent_t *res_dirent = kmalloc(fs->sectors_per_cluster * NBYTES_PER_SECTOR * length);

  // TODO: read in the whole directory (see `read_cluster_chain`)
  // unimplemented();
  read_cluster_chain(fs, cluster_start, res_dirent);

  int index=0, cnt = 0;
  while(res_dirent[index].filename[0] != 0x0) {
    if(res_dirent[index].filename[0] != 0xE5) {
      // printk("res_dirent[index].filename[0] is %x\n", res_dirent[index].filename[0]);
      cnt++;
    }
    index++;
  }
  *dir_n = cnt;
  
  return (fat32_dirent_t *){res_dirent};
}

pi_directory_t fat32_readdir(fat32_fs_t *fs, pi_dirent_t *dirent) {
  demand(init_p, "fat32 not initialized!");
  demand(dirent->is_dir_p, "tried to readdir a file!");
  // TODO: use `get_dirents` to read the raw dirent structures from the disk
  uint32_t n_dirents;
  fat32_dirent_t *dirents = get_dirents(fs, dirent->cluster_id, &n_dirents);
  printk("the n_dirents is %d\n", n_dirents);

  // TODO: allocate space to store the pi_dirent_t return values
  // unimplemented();
  pi_dirent_t *direntArr = kmalloc(n_dirents * sizeof(pi_dirent_t));

  // TODO: iterate over the directory and create pi_dirent_ts for every valid
  // file.  Don't include empty dirents, LFNs, or Volume IDs.  You can use
  // `dirent_convert`.
  // unimplemented();
  int index = 0, dirent_index=0;
  while(dirents[index].filename[0] != 0x0) {
    if(dirents[index].filename[0] != 0xE5) {
      direntArr[dirent_index] = dirent_convert(dirents + index);
      dirent_index++;
    }
    index++;
  }

  // TODO: create a pi_directory_t using the dirents and the number of valid
  // dirents we found
  return (pi_directory_t) {
    .dirents = direntArr,
    .ndirents = n_dirents,
  };
}

int strcasecmp(const char* s1, const char* s2) {
    while (*s1 && *s2) {
        char c1 = (*s1 >= 'A' && *s1 <= 'Z') ? (*s1 + 32) : *s1;
        char c2 = (*s2 >= 'A' && *s2 <= 'Z') ? (*s2 + 32) : *s2;
        if (c1 != c2) {
            return (c1 < c2) ? -1 : 1;
        }
        s1++;
        s2++;
    }
    return (*s1 == *s2) ? 0 : ((*s1 < *s2) ? -1 : 1);
}

// static int find_dirent_with_name(fat32_dirent_t *dirents, int n, char *filename) {
static int find_dirent_with_name(pi_dirent_t *dirents, int n, char *filename) {
  // TODO: iterate through the dirents, looking for a file which matches the
  // name; use `fat32_dirent_name` to convert the internal name format to a
  // normal string.
  // unimplemented();
  for(int i = 0; i < n; i++) {
      pi_dirent_t *d = dirents+i;
      if(strcasecmp(filename, d->name) == 0)
          return i;
      
  }
  return -1;
}

pi_dirent_t *fat32_stat(fat32_fs_t *fs, pi_dirent_t *directory, char *filename) {
  demand(init_p, "fat32 not initialized!");
  demand(directory->is_dir_p, "tried to use a file as a directory");

  // TODO: use `get_dirents` to read the raw dirent structures from the disk
  // unimplemented();
  pi_directory_t pdt = fat32_readdir(fs, directory);

  // TODO: Iterate through the directory's entries and find a dirent with the
  // provided name.  Return NULL if no such dirent exists.  You can use
  // `find_dirent_with_name` if you've implemented it.
  // unimplemented();
  int index = find_dirent_with_name(pdt.dirents, pdt.ndirents, filename);

  // TODO: allocate enough space for the dirent, then convert
  // (`dirent_convert`) the fat32 dirent into a Pi dirent.
  pi_dirent_t *dirent = pdt.dirents + index;
  printk("the index of dirent is %d\n", index);
  printk("the dirent->cluster_id is %d\n", dirent->cluster_id);

  return dirent;
}

pi_file_t *fat32_read(fat32_fs_t *fs, pi_dirent_t *directory, char *filename) {
  // This should be pretty similar to readdir, but simpler.
  demand(init_p, "fat32 not initialized!");
  demand(directory->is_dir_p, "tried to use a file as a directory!");

  // TODO: read the dirents of the provided directory and look for one matching the provided name
  // unimplemented();
  pi_dirent_t *dirent = fat32_stat(fs, directory, filename);

  // TODO: figure out the length of the cluster chain
  // unimplemented();
  uint32_t len = get_cluster_chain_length(fs, dirent->cluster_id);

  // TODO: allocate a buffer large enough to hold the whole file
  // unimplemented();
  char *data = kmalloc(len);

  // TODO: read in the whole file (if it's not empty)
  // unimplemented();
  pi_sd_read(data, cluster_to_lba(fs, dirent->cluster_id), len);

  // TODO: fill the pi_file_t
  pi_file_t *file = kmalloc(sizeof(pi_file_t));
  *file = (pi_file_t) {
    .data = data,
    .n_data = dirent->nbytes,
    .n_alloc = len,
  };
  return file;
}

/******************************************************************************
 * Everything below here is for writing to the SD card (Part 7/Extension).  If
 * you're working on read-only code, you don't need any of this.
 ******************************************************************************/

static uint32_t find_free_cluster(fat32_fs_t *fs, uint32_t start_cluster) {
  // TODO: loop through the entries in the FAT until you find a free one
  // (fat32_fat_entry_type == FREE_CLUSTER).  Start from cluster 3.  Panic if
  // there are none left.
  if (start_cluster < 3) start_cluster = 3;
  unimplemented();
  if (trace_p) trace("failed to find free cluster from %d\n", start_cluster);
  panic("No more clusters on the disk!\n");
}

static void write_fat_to_disk(fat32_fs_t *fs) {
  // TODO: Write the FAT to disk.  In theory we should update every copy of the
  // FAT, but the first one is probably good enough.  A good OS would warn you
  // if the FATs are out of sync, but most OSes just read the first one without
  // complaining.
  if (trace_p) trace("syncing FAT\n");
  unimplemented();

}

// Given the starting cluster index, write the data in `data` over the
// pre-existing chain, adding new clusters to the end if necessary.
static void write_cluster_chain(fat32_fs_t *fs, uint32_t start_cluster, uint8_t *data, uint32_t nbytes) {
  // Walk the cluster chain in the FAT, writing the in-memory data to the
  // referenced clusters.  If the data is longer than the cluster chain, find
  // new free clusters and add them to the end of the list.
  // Things to consider:
  //  - what if the data is shorter than the cluster chain?
  //  - what if the data is longer than the cluster chain?
  //  - the last cluster needs to be marked LAST_CLUSTER
  //  - when do we want to write the updated FAT to the disk to prevent
  //  corruption?
  //  - what do we do when nbytes is 0?
  //  - what about when we don't have a valid cluster to start with?
  //
  //  This is the main "write" function we'll be using; the other functions
  //  will delegate their writing operations to this.

  // TODO: As long as we have bytes left to write and clusters to write them
  // to, walk the cluster chain writing them out.
  unimplemented();

  // TODO: If we run out of clusters to write to, find free clusters using the
  // FAT and continue writing the bytes out.  Update the FAT to reflect the new
  // cluster.
  unimplemented();

  // TODO: If we run out of bytes to write before using all the clusters, mark
  // the final cluster as "LAST_CLUSTER" in the FAT, then free all the clusters
  // later in the chain.
  unimplemented();

  // TODO: Ensure that the last cluster in the chain is marked "LAST_CLUSTER".
  // The one exception to this is if we're writing 0 bytes in total, in which
  // case we don't want to use any clusters at all.
  unimplemented();
}

int fat32_rename(fat32_fs_t *fs, pi_dirent_t *directory, char *oldname, char *newname) {
  // TODO: Get the dirents `directory` off the disk, and iterate through them
  // looking for the file.  When you find it, rename it and write it back to
  // the disk (validate the name first).  Return 0 in case of an error, or 1
  // on success.
  // Consider:
  //  - what do you do when there's already a file with the new name?
  demand(init_p, "fat32 not initialized!");
  if (trace_p) trace("renaming %s to %s\n", oldname, newname);
  if (!fat32_is_valid_name(newname)) return 0;

  // TODO: get the dirents and find the right one
  unimplemented();

  // TODO: update the dirent's name
  unimplemented();

  // TODO: write out the directory, using the existing cluster chain (or
  // appending to the end); implementing `write_cluster_chain` will help
  unimplemented();
  return 1;

}

// Create a new directory entry for an empty file (or directory).
pi_dirent_t *fat32_create(fat32_fs_t *fs, pi_dirent_t *directory, char *filename, int is_dir) {
  demand(init_p, "fat32 not initialized!");
  if (trace_p) trace("creating %s\n", filename);
  if (!fat32_is_valid_name(filename)) return NULL;

  // TODO: read the dirents and make sure there isn't already a file with the
  // same name
  unimplemented();

  // TODO: look for a free directory entry and use it to store the data for the
  // new file.  If there aren't any free directory entries, either panic or
  // (better) handle it appropriately by extending the directory to a new
  // cluster.
  // When you find one, update it to match the name and attributes
  // specified; set the size and cluster to 0.
  unimplemented();

  // TODO: write out the updated directory to the disk
  unimplemented();

  // TODO: convert the dirent to a `pi_dirent_t` and return a (kmalloc'ed)
  // pointer
  unimplemented();
  pi_dirent_t *dirent = NULL;
  return dirent;
}

// Delete a file, including its directory entry.
int fat32_delete(fat32_fs_t *fs, pi_dirent_t *directory, char *filename) {
  demand(init_p, "fat32 not initialized!");
  if (trace_p) trace("deleting %s\n", filename);
  if (!fat32_is_valid_name(filename)) return 0;
  // TODO: look for a matching directory entry, and set the first byte of the
  // name to 0 to mark it as free
  unimplemented();

  // TODO: free the clusters referenced by this dirent
  unimplemented();

  // TODO: write out the updated directory to the disk
  unimplemented();
  return 0;
}

int fat32_truncate(fat32_fs_t *fs, pi_dirent_t *directory, char *filename, unsigned length) {
  demand(init_p, "fat32 not initialized!");
  if (trace_p) trace("truncating %s\n", filename);

  // TODO: edit the directory entry of the file to list its length as `length` bytes,
  // then modify the cluster chain to either free unused clusters or add new
  // clusters.
  //
  // Consider: what if the file we're truncating has length 0? what if we're
  // truncating to length 0?
  unimplemented();

  // TODO: write out the directory entry
  unimplemented();
  return 0;
}

int fat32_write(fat32_fs_t *fs, pi_dirent_t *directory, char *filename, pi_file_t *file) {
  demand(init_p, "fat32 not initialized!");
  demand(directory->is_dir_p, "tried to use a file as a directory!");

  // TODO: Surprisingly, this one should be rather straightforward now.
  // - load the directory
  // - exit with an error (0) if there's no matching directory entry
  // - update the directory entry with the new size
  // - write out the file as clusters & update the FAT
  // - write out the directory entry
  // Special case: the file is empty to start with, so we need to update the
  // start cluster in the dirent

  unimplemented();
}

int fat32_flush(fat32_fs_t *fs) {
  demand(init_p, "fat32 not initialized!");
  // no-op
  return 0;
}
