#include "scheduler.h"

queue* tasks;

//TODO let's squitch over for a task struct for clarity. We can store filenames + status of a task. See node.h.

void scheduler(node* workers, int fd) {
  //Get any files, etc from interface
  printf("The fd is %i\n", fd);
  char* file_name = getBinaryFile(fd, "test");

  if (! file_name)
    return;

  //Create task struct. Add file name
  task* to_do = malloc(sizeof(task));
  to_do->file_name = file_name;
  fprintf(stderr, "%s\n", file_name);

  //Schedule the task
  schedule_task(to_do);

  //See if we can distibute the task
  get_task_from_queue_onto_nodes(workers);
}

void schedule_task(task* work) {
    if (tasks == NULL)
        tasks = queue_create();
    queue_push(tasks, work);
}

void get_task_from_queue_onto_nodes(node* workers) {
    while (!queue_empty(tasks)) {
        task* work = queue_pull(tasks);
        if (distribute_task(workers, work) == 0) {
            break;
        }
    }
}

int distribute_task(node* workers, task* work) {
  node* the_one = get_least_used_worker(workers);
  if (the_one == NULL) {
      queue_push(tasks, work);
      return 0;
  } else {
      for (size_t i=0; i<5; i++) {
         if (the_one->task_list[i] == NULL) {
             the_one->task_list[i] = work;
             set_num_of_task(the_one, get_num_of_task(the_one)+1);
             return 1;
         }
      }
  }
  return 0;
}

//Put any tasks the woker was working on back onto the queue
//"Recover" the tasks
void recover_tasks(node* worker) {
  if (!worker)
    return;

  for (int i = 0; i < MAX_TASKS_PER_NODE; i++) {
      queue_push(tasks, worker->task_list[i]);
      return;
  }
}


void remove_tasks(node* worker, task* work) {
  if (!worker)
    return;

  for (int i = 0; i < MAX_TASKS_PER_NODE; i++) {
    if (worker->task_list[i] == work) {
      //Free task struct here
      worker->task_list[i] = NULL;
      return;
    }
  }
}

//Returns the worker node that is least used
node* get_least_used_worker(node* workers) {

    node* cur = workers;
    node* least_used = NULL;
    double load_factor;
    double least_load_factor = 1000;

    while (cur != NULL) {
        if (is_alive(cur)) {
            continue;
        }
        int num = get_num_of_task(cur);
        if (num >= 5) {
            continue;
        }
        load_factor = (cur->cur_load) + (num * 2);
        if (load_factor < least_load_factor) {
            least_load_factor = load_factor;
            least_used = cur;
        }
        cur = cur->next;
    }

    return cur;
}

void shutdown_scheduler() {
    queue_destroy(tasks);
    tasks = NULL;
}


ssize_t read_all_from_socket(int socket, char *buffer, size_t count) {
    ssize_t curr_read;
    ssize_t total_read = 0;
    errno = 0;

    while(count) {
      curr_read = read(socket, buffer, count);
      if (curr_read == -1) {
        if (errno == EINTR)
          continue;
        return -1;
      } else if (curr_read == 0)
        break;
      count -= curr_read;
      total_read += curr_read;
      buffer += curr_read;
    }

    return total_read;
}

ssize_t write_all_to_socket(int socket, const char *buffer, size_t count) {
    ssize_t curr_write;
    ssize_t total_write = 0;
    errno = 0;

    while(count) {
      curr_write = write(socket, buffer, count);
      if (curr_write == -1) {
        if (errno == EINTR)
          continue;
        return -1;
      } else if (curr_write == 0)
        break;
      count -= curr_write;
      total_write += curr_write;
      buffer += curr_write;
    }

    return total_write;
}

ssize_t write_all_from_socket_to_fd(int socket, int fd, ssize_t size) {
  char buffer[4096];
  ssize_t curr_read;
  ssize_t total_read = 0;
  errno = 0;
  ssize_t initial = size;
  while (size) {
    curr_read = (4096 < size || initial == -1) ? read_all_from_socket(socket, buffer, 4096) : read_all_from_socket(socket, buffer, size);
    // printf("curr_read is %zi\n", curr_read);
    if (curr_read == -1) {
      // printf("%s\n", strerror(errno));
      if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)
        continue;
      // else if (errno == EWOULDBLOCK || errno == EAGAIN)
      //   return NOT_DONE_SENDING;
      return -1;
    } else if (curr_read == 0) {
        break;
    }
    size -= curr_read;
    total_read += curr_read;

    ssize_t written = write_all_to_socket(fd, buffer, curr_read);

    if (written == - 1 && errno == EPIPE)
      return -1;
  }

  return total_read;
}
