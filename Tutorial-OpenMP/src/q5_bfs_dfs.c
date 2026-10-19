/**
 * Tutorial-05 OpenMP: Question 5
 * Implement BFS and DFS traversals using OpenMP with guaranteed thread safety.
 *
 * Strategies:
 * 1) Parallel BFS: Level-synchronous frontier parallelization with atomic test-and-set
 *    on visited flags and atomic queue indexing.
 * 2) Parallel DFS: OpenMP task-based traversal with depth cutoff and atomic visited marking.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <omp.h>

typedef struct Graph {
    int num_vertices;
    int num_edges;
    int* head;
    int* to;
    int* next;
} Graph;

// Create graph with adjacency list
Graph* create_graph(int v, int max_edges) {
    Graph* g = (Graph*)malloc(sizeof(Graph));
    g->num_vertices = v;
    g->num_edges = 0;
    g->head = (int*)malloc(v * sizeof(int));
    memset(g->head, -1, v * sizeof(int));
    g->to = (int*)malloc(max_edges * sizeof(int));
    g->next = (int*)malloc(max_edges * sizeof(int));
    return g;
}

void add_edge(Graph* g, int u, int v) {
    int e = g->num_edges++;
    g->to[e] = v;
    g->next[e] = g->head[u];
    g->head[u] = e;
}

void free_graph(Graph* g) {
    if (!g) return;
    free(g->head);
    free(g->to);
    free(g->next);
    free(g);
}

// Generate a random connected graph with branch factor
Graph* generate_benchmark_graph(int v, int avg_deg) {
    int max_edges = v * avg_deg * 2 + v * 4;
    Graph* g = create_graph(v, max_edges);

    // Ensure connectivity: connect to parent
    for (int i = 1; i < v; i++) {
        int parent = i / 2;
        add_edge(g, parent, i);
        add_edge(g, i, parent);
    }
    // Add additional random edges
    for (int i = 0; i < v * avg_deg; i++) {
        int u = rand() % v;
        int w = rand() % v;
        if (u != w) {
            add_edge(g, u, w);
        }
    }
    return g;
}

// ====================================================================
// BFS Implementations
// ====================================================================

// Sequential BFS
int bfs_sequential(Graph* g, int start_node, double* elapsed) {
    int v = g->num_vertices;
    int* visited = (int*)calloc(v, sizeof(int));
    int* queue = (int*)malloc(v * sizeof(int));
    int head = 0, tail = 0;

    double t0 = omp_get_wtime();
    visited[start_node] = 1;
    queue[tail++] = start_node;

    while (head < tail) {
        int u = queue[head++];
        for (int e = g->head[u]; e != -1; e = g->next[e]) {
            int nxt = g->to[e];
            if (!visited[nxt]) {
                visited[nxt] = 1;
                queue[tail++] = nxt;
            }
        }
    }
    *elapsed = omp_get_wtime() - t0;
    int count = tail;
    free(visited);
    free(queue);
    return count;
}

// Parallel Level-Synchronous BFS
int bfs_parallel(Graph* g, int start_node, int num_threads, double* elapsed) {
    int v = g->num_vertices;
    int* visited = (int*)calloc(v, sizeof(int));
    int* curr_frontier = (int*)malloc(v * sizeof(int));
    int* next_frontier = (int*)malloc(v * sizeof(int));

    int curr_size = 0;
    int next_size = 0;
    int total_visited = 0;

    double t0 = omp_get_wtime();
    visited[start_node] = 1;
    curr_frontier[curr_size++] = start_node;
    total_visited = 1;

    omp_set_num_threads(num_threads);

    while (curr_size > 0) {
        next_size = 0;

        #pragma omp parallel
        {
            #pragma omp for schedule(guided)
            for (int i = 0; i < curr_size; i++) {
                int u = curr_frontier[i];
                for (int e = g->head[u]; e != -1; e = g->next[e]) {
                    int nxt = g->to[e];
                    // Atomic test-and-set to guarantee unique visited state
                    if (__sync_bool_compare_and_swap(&visited[nxt], 0, 1)) {
                        int pos;
                        #pragma omp atomic capture
                        pos = next_size++;
                        next_frontier[pos] = nxt;
                    }
                }
            }
        }

        total_visited += next_size;

        // Swap frontiers
        int* tmp = curr_frontier;
        curr_frontier = next_frontier;
        next_frontier = tmp;
        curr_size = next_size;
    }

    *elapsed = omp_get_wtime() - t0;
    free(visited);
    free(curr_frontier);
    free(next_frontier);
    return total_visited;
}

// ====================================================================
// DFS Implementations
// ====================================================================

// Sequential DFS Helper
void dfs_seq_kernel(Graph* g, int u, int* visited, int* count) {
    visited[u] = 1;
    (*count)++;
    for (int e = g->head[u]; e != -1; e = g->next[e]) {
        int nxt = g->to[e];
        if (!visited[nxt]) {
            dfs_seq_kernel(g, nxt, visited, count);
        }
    }
}

int dfs_sequential(Graph* g, int start_node, double* elapsed) {
    int v = g->num_vertices;
    int* visited = (int*)calloc(v, sizeof(int));
    int count = 0;

    double t0 = omp_get_wtime();
    dfs_seq_kernel(g, start_node, visited, &count);
    *elapsed = omp_get_wtime() - t0;

    free(visited);
    return count;
}

// Parallel DFS Task-based Kernel
void dfs_par_kernel(Graph* g, int u, int* visited, int* count, int depth, int cutoff) {
    #pragma omp atomic
    (*count)++;

    for (int e = g->head[u]; e != -1; e = g->next[e]) {
        int nxt = g->to[e];
        // Thread-safe atomic test-and-set
        if (__sync_bool_compare_and_swap(&visited[nxt], 0, 1)) {
            if (depth < cutoff) {
                #pragma omp task
                dfs_par_kernel(g, nxt, visited, count, depth + 1, cutoff);
            } else {
                dfs_par_kernel(g, nxt, visited, count, depth + 1, cutoff);
            }
        }
    }
}

int dfs_parallel(Graph* g, int start_node, int num_threads, int cutoff, double* elapsed) {
    int v = g->num_vertices;
    int* visited = (int*)calloc(v, sizeof(int));
    int count = 0;

    double t0 = omp_get_wtime();
    visited[start_node] = 1;

    #pragma omp parallel num_threads(num_threads)
    {
        #pragma omp single
        {
            dfs_par_kernel(g, start_node, visited, &count, 0, cutoff);
        }
    }

    *elapsed = omp_get_wtime() - t0;
    free(visited);
    return count;
}

int main(int argc, char* argv[]) {
    int num_vertices = 30000;
    int avg_degree = 4;
    int cutoff = 10;

    if (argc > 1) num_vertices = atoi(argv[1]);
    if (argc > 2) avg_degree = atoi(argv[2]);

    printf("====================================================================\n");
    printf(" OpenMP Tutorial 05 - Question 5: Parallel BFS & DFS Traversals\n");
    printf("====================================================================\n");
    printf("Vertices: %d | Avg Degree: %d\n", num_vertices, avg_degree);

    printf("Generating graph in memory... ");
    fflush(stdout);
    srand(42);
    Graph* g = generate_benchmark_graph(num_vertices, avg_degree);
    printf("Done. Total directed edges: %d\n\n", g->num_edges);

    // 1. BFS Comparison
    double t_seq_bfs = 0.0, t_par_bfs = 0.0;
    int visited_seq_bfs = bfs_sequential(g, 0, &t_seq_bfs);

    printf("--- Breadth-First Search (BFS) Traversal ---\n");
    printf("Sequential BFS: Visited = %d | Time = %.4f s\n", visited_seq_bfs, t_seq_bfs);

    int thread_counts[] = {1, 2, 4, 8, 16};
    int num_tc = sizeof(thread_counts) / sizeof(thread_counts[0]);

    printf("%-8s | %-12s | %-12s | %-10s | %-8s\n", 
           "Threads", "Visited", "Time (s)", "Speedup", "Status");
    printf("-----------------------------------------------------------------\n");
    for (int i = 0; i < num_tc; i++) {
        int th = thread_counts[i];
        int v_par = bfs_parallel(g, 0, th, &t_par_bfs);
        double sp = (t_seq_bfs > 0) ? (t_seq_bfs / t_par_bfs) : 1.0;
        printf("%-8d | %-12d | %-12.4f | %-10.2fx | %-8s\n",
               th, v_par, t_par_bfs, sp, (v_par == visited_seq_bfs ? "PASSED" : "FAILED"));
    }
    printf("\n");

    // 2. DFS Comparison
    double t_seq_dfs = 0.0, t_par_dfs = 0.0;
    int visited_seq_dfs = dfs_sequential(g, 0, &t_seq_dfs);

    printf("--- Depth-First Search (DFS) Traversal ---\n");
    printf("Sequential DFS: Visited = %d | Time = %.4f s\n", visited_seq_dfs, t_seq_dfs);

    printf("%-8s | %-12s | %-12s | %-10s | %-8s\n", 
           "Threads", "Visited", "Time (s)", "Speedup", "Status");
    printf("-----------------------------------------------------------------\n");
    for (int i = 0; i < num_tc; i++) {
        int th = thread_counts[i];
        int v_par = dfs_parallel(g, 0, th, cutoff, &t_par_dfs);
        double sp = (t_seq_dfs > 0) ? (t_seq_dfs / t_par_dfs) : 1.0;
        printf("%-8d | %-12d | %-12.4f | %-10.2fx | %-8s\n",
               th, v_par, t_par_dfs, sp, (v_par == visited_seq_dfs ? "PASSED" : "FAILED"));
    }

    printf("-----------------------------------------------------------------\n");
    printf("Thread-Safety: Guaranteed via atomic CAS (__sync_bool_compare_and_swap).\n");
    printf("====================================================================\n");

    free_graph(g);
    return 0;
}
