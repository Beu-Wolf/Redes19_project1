#include "commands.h"
#include <fcntl.h>

/* The main purpose of this code is to prevent
 * answers being submitted at the same time to get the same number.
 * Submitting answers should be an atomic operation.
 */

/* TODO: remove debug messages */

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

    fprintf(stderr, "[%d] Waiting for lock...\n", getpid());
    /* Retry getting the lock if interrupted */
    while(fcntl(lockfd, F_SETLKW, &lock) == -1 && errno == EINTR);
    fprintf(stderr, "[%d] Lock acquired\n", getpid());
}

void questionUnlock(void) {
    lock.l_type = F_UNLCK;
    fprintf(stderr, "[%d] Unlocking...\n", getpid());
    fcntl(lockfd, F_SETLK, &lock);
    fprintf(stderr, "[%d] Unlocked...\n", getpid());

    close(lockfd);
}
