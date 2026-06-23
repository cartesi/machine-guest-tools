#ifndef CMT_UTIL_H
#define CMT_UTIL_H
#include <stdbool.h>
#include <stddef.h>

/** Wraps a libcmt call for debug tracing. When @ref CMT_DEBUG=yes and the
 *  inner expression returns non-zero, prints file/line/expression/error to
 *  stderr.  Always returns the result of @p X unchanged, so it composes
 *  cleanly in if-chains.
 *
 *  @code
 *  if (CMT_DBG(cmt_abi_put_funsel(wr, MY_FUNSEL))
 *  ||  CMT_DBG(cmt_abi_put_uint256(wr, &val))) {
 *      return -ENOBUFS;
 *  }
 *  @endcode */
#define CMT_DBG(X) cmt_util_debug((X), #X, __FILE__, __LINE__)

/** Internal: do not call directly. Use @ref CMT_DBG. */
int cmt_util_debug(int rc, const char *expr, const char *file, int line);

/**
 */
bool cmt_util_debug_enabled(void);

/** Read whole file `name` contents into `data` and set `length`.
 * @param name[in]    - file path
 * @param max[in]     - size of `data` in bytes
 * @param data[out]   - file contents
 * @param length[out] - actual size in `bytes` written to `data`
 *
 * @return
 * |     |                    |
 * |-----|--------------------|
 * |   0 |success             |
 * | < 0 |negative errno value| */
int cmt_util_read_whole_file(const char *name, size_t max, void *data, size_t *length);

/** Write the contents of `data` into file `name`.
 * @param name[in]    - file path
 * @param length[in]  - size of `data` in bytes
 * @param data[out]   - file contents
 *
 * @return
 * |     |                    |
 * |-----|--------------------|
 * |   0 |success             |
 * | < 0 |negative errno value| */
int cmt_util_write_whole_file(const char *name, size_t length, const void *data);

#endif /* CMT_UTIL_H */
