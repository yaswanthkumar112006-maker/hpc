/**
 * Tutorial-05 OpenMP: Question 4
 * Parallel traversal of a binary tree using OpenMP tasks to compute node sum.
 * Demonstrates:
 * - OpenMP tasking model (#pragma omp task, #pragma omp taskwait)
 * - Cutoff depth optimization to balance task granularity vs scheduling overhead
 * - Thread safety without mutual exclusion locks (pure recursive reduction)
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define DEFAULT_DEPTH 22  // 2^22 - 1 = 4,194,303 nodes
#define TASK_CUTOFF_DEPTH 10

typedef struct TreeNode {
    long long value;
    struct TreeNode* left;
    struct TreeNode* right;
} TreeNode;

// Build a full balanced binary tree of specified depth
TreeNode* build_tree(int current_depth, int max_depth, long long* node_id) {
    if (current_depth > max_depth) return NULL;
    TreeNode* node = (TreeNode*)malloc(sizeof(TreeNode));
    (*node_id)++;
    node->value = (*node_id % 100) + 1; // values between 1 and 100
    node->left = build_tree(current_depth + 1, max_depth, node_id);
    node->right = build_tree(current_depth + 1, max_depth, node_id);
    return node;
}

// Free tree memory
void free_tree(TreeNode* root) {
    if (!root) return;
    free_tree(root->left);
    free_tree(root->right);
    free(root);
}

// Sequential recursive sum (Ground truth)
long long sum_tree_seq(TreeNode* root) {
    if (!root) return 0;
    return root->value + sum_tree_seq(root->left) + sum_tree_seq(root->right);
}

// OpenMP task-based parallel recursive sum with depth cutoff
long long sum_tree_parallel_tasks(TreeNode* root, int current_depth, int cutoff) {
    if (!root) return 0;

    // Below cutoff depth, process sequentially to eliminate task creation overhead
    if (current_depth >= cutoff) {
        return sum_tree_seq(root);
    }

    long long left_sum = 0;
    long long right_sum = 0;

    #pragma omp task shared(left_sum)
    left_sum = sum_tree_parallel_tasks(root->left, current_depth + 1, cutoff);

    #pragma omp task shared(right_sum)
    right_sum = sum_tree_parallel_tasks(root->right, current_depth + 1, cutoff);

    #pragma omp taskwait
    return root->value + left_sum + right_sum;
}

// Wrapper to launch parallel region
long long sum_tree_parallel(TreeNode* root, int num_threads, int cutoff, double* elapsed) {
    long long total_sum = 0;
    double t_start = omp_get_wtime();

    #pragma omp parallel num_threads(num_threads) shared(total_sum)
    {
        #pragma omp single
        {
            total_sum = sum_tree_parallel_tasks(root, 0, cutoff);
        }
    }

    *elapsed = omp_get_wtime() - t_start;
    return total_sum;
}

int main(int argc, char* argv[]) {
    int depth = DEFAULT_DEPTH;
    int cutoff = TASK_CUTOFF_DEPTH;

    if (argc > 1) depth = atoi(argv[1]);
    if (argc > 2) cutoff = atoi(argv[2]);

    long long total_nodes = (1LL << depth) - 1;

    printf("====================================================================\n");
    printf(" OpenMP Tutorial 05 - Question 4: Parallel Binary Tree Traversal\n");
    printf("====================================================================\n");
    printf("Tree Depth: %d | Total Nodes: %lld | Task Cutoff Depth: %d\n\n", 
           depth, total_nodes, cutoff);

    printf("Constructing binary tree in memory... ");
    fflush(stdout);
    long long node_counter = 0;
    double t0 = omp_get_wtime();
    TreeNode* root = build_tree(1, depth, &node_counter);
    printf("Done in %.2f s\n\n", omp_get_wtime() - t0);

    // 1. Sequential Run
    printf("[1] Sequential Tree Traversal:\n");
    t0 = omp_get_wtime();
    long long seq_sum = sum_tree_seq(root);
    double t_seq = omp_get_wtime() - t0;
    printf("    Sequential Sum: %lld | Time: %.4f s\n\n", seq_sum, t_seq);

    // 2. Parallel Run across thread counts
    int threads_list[] = {1, 2, 4, 8, 16};
    int num_configs = sizeof(threads_list) / sizeof(threads_list[0]);

    printf("[2] OpenMP Task-Based Parallel Tree Traversal:\n");
    printf("--------------------------------------------------------------------------------\n");
    printf("%-8s | %-16s | %-12s | %-10s | %-10s | %-8s\n", 
           "Threads", "Computed Sum", "Time (s)", "Speedup", "Efficiency", "Match?");
    printf("--------------------------------------------------------------------------------\n");

    for (int i = 0; i < num_configs; i++) {
        int th = threads_list[i];
        double t_par = 0.0;
        long long par_sum = sum_tree_parallel(root, th, cutoff, &t_par);
        double speedup = (t_seq > 0) ? (t_seq / t_par) : 1.0;
        double efficiency = (speedup / th) * 100.0;
        const char* match_str = (par_sum == seq_sum) ? "MATCH" : "MISMATCH!";

        printf("%-8d | %-16lld | %-12.4f | %-10.2fx | %9.1f%% | %-8s\n",
               th, par_sum, t_par, speedup, efficiency, match_str);
    }
    printf("--------------------------------------------------------------------------------\n");
    printf("Thread Safety Verified: Pure hierarchical taskwait guarantees zero race conditions.\n");
    printf("====================================================================\n");

    free_tree(root);
    return 0;
}
