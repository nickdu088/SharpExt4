#pragma once

namespace SharpExt4 {
    enum class EntryType{
        UNKNOWN = 0,
        REG_FILE,
        DIR,
        CHRDEV,
        BLKDEV,
        FIFO,
        SOCK,
        SYMLINK
    };
}

