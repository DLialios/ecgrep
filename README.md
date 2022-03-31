Made primarily for use with fd and xargs to recursively search within repos

`lookfor() { fd -tf | xargs -P 0 ecgrep "$1" }`
