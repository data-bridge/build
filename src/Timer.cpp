



// http://stackoverflow.com/questions/17925051/fast-textfile-reading-in-c
//
static uintmax_t wc(char const *fname)
{
  static const auto BUFFER_SIZE = 16*1024;
  int fd = open(fname, O_RDONLY);
  if(fd == -1)
    handle_error("open");

  /* Advise the kernel of our access pattern.  */
  posix_fadvise(fd, 0, 0, 1);  // FDADVICE_SEQUENTIAL

  char buf[BUFFER_SIZE + 1];
  uintmax_t lines = 0;

  while(size_t bytes_read = read(fd, buf, BUFFER_SIZE))
  {
    if(bytes_read == (size_t)-1)
      handle_error("read failed");
    if (!bytes_read)
      break;

    for(char *p = buf; (p = (char*) memchr(p, '\n', (buf + bytes_read) - p)); ++p)
      ++lines;
  }

  return lines;
}
