# vim: fdm=indent
#find dest -exec stat "{}" -c '("%n", %i),' \;

TELEMETRY = [
    ("bytes_per_sector", 512),
    ("sectors_per_cluster", 8),
    ("reserved_sectors", 32),
    ("fat_num_entries", 127872),
    ("fat_begin_offset", 16384),
]

START_CLUSTER_BASIC = [
    (".", 2),
    ("LARGE1", 783),
    ("2READ.ING", 259),
    ("1GOOD.JOB", 260),
    ("SUB.DIR", 265),
    ("BIGDIR", 275),
    ("EMPTY", 0),
]

START_CLUSTER_SHORT = [
]


START_CLUSTER_LONG = [
]

START_CLUSTER_BIGDIR_LONG = [
]

START_CLUSTER_BIGDIR_SHORT = [
]

# Get with
# md5sum -b reference/* | sed 's/\([0-9a-z]*\) .*\/\([0-9.a-zA-Z! ]*\)/\("\2", "\1"\),/'
BASIC_MD5_SUMS = [
    ("1GOOD.JOB", "7d3f994166cc5ffad1fbc954bf408d53"),
    ("2READ.ING", "52470081c96fa1747695271777761b28"),
    ("3THE", "0ff454b3c367476a7cac82b33e3d4aca"),
    ("4DIR!", "b3d2495650ac5d25b055a895207a90d1"),
    ("EMPTY", "d41d8cd98f00b204e9800998ecf8427e"),

    ("LARGE1", "fa7ce78992a8f4e646d4e423315afc5e"),
    ("LARGE2", "6da8d970c176ead5add599fe01f3e783"),
    
    ("RAND.OM", "802379856cdf5f2185bba49ca3b717a8"),
]

MORE_MD5_SUMS_SHORT = [
]

MORE_MD5_SUMS_LONG = [
]

BIGDIR_MD5_SUMS_SHORT = [
]

BIGDIR_MD5_SUMS_LONG = [
]

NEXT_CLUSTERS = [
    (1, 268435455),
    (3, 0),
    (274, 268435455),
    (275, 339),
    (276, 268435455),
    (800, 801),
    (4747, 0),
]


# find dest -exec stat "{}" -c '("%n", %X, %Y, %Z),' \;
TIMESTAMP_BASIC = [
    ("BIGDIR", 1273532400, 1273575768, 1273575768),
    ("EMPTY", 1273532400, 1273575768, 1273575768),
    ("RAND.OM", 1273532400, 1273581502, 1273581502),
    ("2READ.ING", 1273532400, 1273575766, 1273575767),
    ("1GOOD.JOB", 1273532400, 1273575766, 1273575767),
    ("3THE", 1273532400, 1273575766, 1273575767),
    ("4DIR!", 1273532400, 1273575766, 1273575767),
]

TIMESTAMP_BIGDIR_LONG = [
]

TIMESTAMP_LONG = [
]
