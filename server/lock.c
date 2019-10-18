#include "commands.h"
#include <fcntl.h>

/* The main purpose of this code is to prevent
 * answers being submitted at the same time to get the same number.
 * Submitting answers should be an atomic operation.
 */

static int lockfd;
static struct flock lock;

void questionLock(char *topic, char *question) {
    char *path = strdup(TOPICSDIR"/");

    path = safestrcat(path, topic);
    path = safestrcat(path, "/");
    path = safestrcat(path, question);
    path = safestrcat(path, "/");
    path = safestrcat(path, LOCKFILE);

    lockfd = open(path, O_WRONLY);

    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;

    /* Retry getting the lock if interrupted */
    while(fcntl(lockfd, F_SETLKW, &lock) == -1 && errno == EINTR);
}

void questionUnlock(void) {
    lock.l_type = F_UNLCK;
    fcntl(lockfd, F_SETLK, &lock);
    close(lockfd);
}
