/**
 * @file query.cpp
 * @author unnamed LLM
 * @brief Just a quick placeholder demo to test if linenoise is properly working after I packed it as meson. It will be removed and replaced by a propert query utility.
 * @version 0.1
 * @date 2025-05-25
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <iostream>
#include <string_view>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <cassert>
#include <memory>
#include "linenoise.h"

// Simple tree structure node
struct Node {
    std::string name;
    std::vector<std::shared_ptr<Node>> children;
    std::weak_ptr<Node> parent; // weak pointer to avoid cycle

    Node(const std::string &n) : name(n) {}
};

// Global pointer to currently navigated node
std::shared_ptr<Node> currentNode = nullptr;

// Root of our tree
std::shared_ptr<Node> root = nullptr;

// Helper: Find a child with the given name in a node
std::shared_ptr<Node> findChild(const std::shared_ptr<Node> &node, const std::string &name) {
    for (auto &child : node->children) {
        if (child->name == name) {
            return child;
        }
    }
    return nullptr;
}

// Build a sample tree
void buildTree() {
    root = std::make_shared<Node>("root");
    currentNode = root;

    // Create some children for root
    auto folder1 = std::make_shared<Node>("folder1");
    auto folder2 = std::make_shared<Node>("folder2");
    auto file1 = std::make_shared<Node>("file1.txt");

    folder1->parent = root;
    folder2->parent = root;
    file1->parent = root;

    root->children.push_back(folder1);
    root->children.push_back(folder2);
    root->children.push_back(file1);

    // Add subitems into folder1
    auto subfile1 = std::make_shared<Node>("subfile1.txt");
    subfile1->parent = folder1;
    folder1->children.push_back(subfile1);

    // Add subitems into folder2
    auto subfolder = std::make_shared<Node>("subfolder");
    subfolder->parent = folder2;
    folder2->children.push_back(subfolder);
}

// Completion callback for linenoise
// It will provide completion for commands "cd" and "ls" and if "cd" is the current command,
// it completes the child node names available in the current node.
void completionCallback(const char *buf, linenoiseCompletions *lc) {
    std::string input(buf);
    // we trim left spaces (a very simple trim)
    input.erase(0, input.find_first_not_of(" \t"));

    std::vector<std::string> tokens;
    std::istringstream iss(input);
    std::string word;
    while (iss >> word) {
        tokens.push_back(word);
    }
    
    // If no tokens yet, then complete with all commands.
    if (tokens.empty()) {
        linenoiseAddCompletion(lc, "ls");
        linenoiseAddCompletion(lc, "cd");
        return;
    }

    // If the command is partial, complete command names.
    if (tokens.size() == 1) {
        std::string part = tokens[0];
        if (std::string_view("ls").find(part) == 0) {
            linenoiseAddCompletion(lc, "ls");
        }
        if (std::string_view("cd").find(part) == 0) {
            linenoiseAddCompletion(lc, "cd");
        }
        return;
    }

    // If the command is "cd", then provide completions for the child names.
    if (tokens[0] == "cd") {
        std::string arg = tokens[1];
        // Provide special support for ".." to go back if possible.
        if (std::string_view("..").find(arg) == 0) {
            linenoiseAddCompletion(lc, "..");
        }
        // List all children whose name starts with arg.
        if (currentNode) {
            for (auto &child : currentNode->children) {
                if (child->name.find(arg) == 0) {
                    linenoiseAddCompletion(lc, child->name.c_str());
                }
            }
        }
    }
}

// Helper function to list the children of current node
void listChildren() {
    if (!currentNode) return;
    std::cout << "Contents of node \"" << currentNode->name << "\":" << std::endl;
    for (auto &child : currentNode->children) {
        std::cout << "  " << child->name << std::endl;
    }
}

// Process user commands
void processCommand(const std::string &line) {
    std::istringstream iss(line);
    std::string command;
    iss >> command;
    
    if (command == "ls") {
        listChildren();
    }
    else if (command == "cd") {
        std::string target;
        iss >> target;
        if (target.empty()) {
            std::cout << "Usage: cd [node name | ..]" << std::endl;
            return;
        }
        if (target == "..") {
            // move to parent if possible
            if (auto parentShared = currentNode->parent.lock()) {
                currentNode = parentShared;
            } else {
                std::cout << "Already at the root node." << std::endl;
            }
        } else {
            auto child = findChild(currentNode, target);
            if (child) {
                currentNode = child;
            } else {
                std::cout << "No such node: " << target << std::endl;
            }
        }
        std::cout << "Now in node: " << currentNode->name << std::endl;
    }
    else if (command == "pwd") {
        // Print full path
        std::vector<std::string> path;
        std::shared_ptr<Node> node = currentNode;
        while (node) {
            path.push_back(node->name);
            node = node->parent.lock();
        }
        std::reverse(path.begin(), path.end());
        std::cout << "/";
        for (auto &p : path) {
            std::cout << p << "/";
        }
        std::cout << std::endl;
    }
    else if (command == "exit") {
        exit(0);
    }
    else {
        std::cout << "Unknown command: " << command << std::endl;
        std::cout << "Commands: ls, cd, pwd, exit" << std::endl;
    }
}

int main() {
    // Build our simple tree
    buildTree();

    // Set the completion callback for linenoise.
    linenoiseSetCompletionCallback(completionCallback);
    
    // Optionally, add multi-line support or hints here
    // linenoiseSetHintsCallback(...); 

    std::cout << "Simple tree navigation. Commands: ls, cd <node>, pwd, exit" << std::endl;
    std::cout << "Use TAB to auto-complete." << std::endl;

    while (true) {
        // Construct the prompt showing current node
        std::string prompt = currentNode ? (currentNode->name + " > ") : "> ";
        const char* line = linenoise(prompt.c_str());
        if (line == nullptr) {
            break; // EOF / Ctrl-D
        }

        // Skip empty lines.
        if (line[0] != '\0') {
            // Add non-empty lines to history
            linenoiseHistoryAdd(line);
            processCommand(line);
        }
        free((void*)line);
    }
    return 0;
}