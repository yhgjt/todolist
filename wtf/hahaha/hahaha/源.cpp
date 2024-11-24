#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <functional>
#include <cstdlib>
#include<fstream>
#include <unordered_set>
// 保存任务到文件


// 时间解析函数
time_t parseDatetime(const std::string& datetime) {
    std::tm tm = {};
    std::istringstream ss(datetime);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S"); // 格式: YYYY-MM-DD HH:MM:SS
    if (ss.fail()) {
        std::cerr << "Invalid datetime format. Please use YYYY-MM-DD HH:MM:SS.\n";
        return -1;
    }
    return mktime(&tm); // 转换为时间戳
}

// 格式化时间戳为人类可读格式
std::string formatDatetime(time_t timestamp) {
    char buffer[26];
    ctime_s(buffer, sizeof(buffer), &timestamp);
    return std::string(buffer);
}
class Tag {
private:
    std::string name;

public:
    Tag(const std::string& name) : name(name) {}

    void setName(const std::string& newName) {
        name = newName;
    }

    std::string getName() const {
        return name;
    }
};
// Reminder 类
class Reminder {
private:
    time_t remindTime;
    std::string method;

public:
    Reminder(time_t time, const std::string& method) : remindTime(time), method(method) {}

    void setRemindTime(time_t time) {
        remindTime = time;
    }

    void setMethod(const std::string& newMethod) {
        method = newMethod;
    }

    time_t getRemindTime() const {
        return remindTime;
    }

    void remind(const std::string& taskName) const {
        std::cout << "Reminder for task '" << taskName << "': " << method
            << " at " << formatDatetime(remindTime) << std::endl;
    }
};

// Task 类
class Task {
private:
    std::string name;
    std::string description;
    time_t ddl;
    int priority;
    std::vector<std::string> tags;
    Reminder reminder;

public:
    Task(const std::string& name, const std::string& description, time_t ddl, int priority, const Reminder& reminder)
        : name(name), description(description), ddl(ddl), priority(priority), reminder(reminder) {}

    void setName(const std::string& newName) {
        name = newName;
    }

    void setDescription(const std::string& newDescription) {
        description = newDescription;
    }

    void setDdl(time_t newDdl) {
        ddl = newDdl;
    }

    void setPriority(int newPriority) {
        priority = newPriority;
    }

    void editTask(const std::string& n, const std::string& d, time_t dd, int p, const Reminder& r)
    {
        name = n;
        description = d;
        ddl = dd;
        priority = p;
        reminder = r;
    }
    void addTag(const std::string& tag) {
        tags.push_back(tag);
    }

    void removeTag(const std::string& tagName) {
        tags.erase(std::remove(tags.begin(), tags.end(), tagName), tags.end());
    }

    const std::string& getName() const {
        return name;
    }

    const std::string& getDescription() const {
        return description;
    }

    int getPriority() const {
        return priority;
    }

    time_t getDdl() const {
        return ddl;
    }

    Reminder getReminder() const {
        return reminder;
    }

    bool hasTag(const std::string& tagName) const {
        return std::any_of(tags.begin(), tags.end(),
            [&](const Tag& t) { return t.getName() == tagName; });
    }

    void display() const {
        std::cout << "Task: " << name
            << "\nDescription: " << description
            << "\nDDL: " << formatDatetime(ddl)
            << "Priority: " << priority
            << "\nTags: ";
        for (const auto& tag : tags) {
            std::cout << tag << " ";
        }
        std::cout << "\nReminder: " << formatDatetime(reminder.getRemindTime()) << "\n";
    }
};
class Category {
private:
    std::string name;
    std::vector<Task> tasks;

public:
    Category(const std::string& name) : name(name) {}

    void setName(const std::string& newName) {
        name = newName;
    }

    void addTask(const Task& task) {
        tasks.push_back(task);
    }

    void removeTask(const std::string& taskName) {
        tasks.erase(std::remove_if(tasks.begin(), tasks.end(),
            [&](const Task& t) { return t.getName() == taskName; }),
            tasks.end());
    }

    bool hasTask(const std::string& taskName)
    {
        for (auto& task : tasks) {
            if (task.getName()==taskName) {
                return true;
            }
        }
        return false;
    }

    std::string getName() const {
        return name;
    }

    void display() const {
        std::cout << "Category: " << name << "\nTasks:\n";
        for (const auto& task : tasks) {
            task.display();
        }
    }
};
// 多线程提醒函数
void runReminder(const std::vector<Task>& tasks) {
    while (true) {
        time_t now = time(nullptr);
        for (const auto& task : tasks) {
            if (task.getReminder().getRemindTime() <= now && task.getReminder().getRemindTime() != 0) {
                task.getReminder().remind(task.getName());
                std::cout << "The time now is : " << formatDatetime(now) << std::endl;
            }
        }
        
        std::this_thread::sleep_for(std::chrono::seconds(30)); // 每 30 秒检查一次
    }
}

// BatchOperation 类
class BatchOperation {
public:
    static void addTagsToTasks(std::vector<Task>& tasks, const std::vector<std::string>& taskNames, const std::string& tagName) {
        for (auto& task : tasks) {
            if (std::find(taskNames.begin(), taskNames.end(), task.getName()) != taskNames.end()) {
                task.addTag(tagName);
            }
        }
        std::cout << "Tag '" << tagName << "' added to selected tasks.\n";
    }
    static void categorizeTasks(std::vector<Task>& tasks, const std::vector<std::string>& taskNames, Category& category) {
        for (auto& task : tasks) {
            if (std::find(taskNames.begin(), taskNames.end(), task.getName()) != taskNames.end()) {
                category.addTask(task);
            }
        }
        std::cout << "Tasks added to category: " << category.getName() << "\n";
    }
    static void deleteTasks(std::vector<Task>& tasks, const std::vector<std::string>& taskNames) {
        // 创建一个任务名称的集合，以提高查找效率
        std::unordered_set<std::string> taskNamesSet(taskNames.begin(), taskNames.end());

        // 用来存储删除的任务数量
        int deletedCount = 0;

        // 遍历任务列表并删除符合条件的任务
        for (auto it = tasks.begin(); it != tasks.end(); ) {
            if (taskNamesSet.find(it->getName()) != taskNamesSet.end()) {
                // 找到匹配的任务，删除它
                it = tasks.erase(it);
                deletedCount++;
            }
            else {
                // 如果没有找到，继续遍历下一个任务
                ++it;
            }
        }

        std::cout << deletedCount << " task(s) deleted.\n";
    }
};

// TaskManager 类
class TaskManager {
private:
    std::vector<Task> tasks;

public:
    void createTask(const std::string& name, const std::string& description, time_t ddl, int priority, const Reminder& reminder) {
        tasks.emplace_back(name, description, ddl, priority, reminder);
        std::cout << "Task created: " << name << std::endl;
    }

    void listTasks() const {
        std::cout << "Listing all tasks:\n";
        for (const auto& task : tasks) {
            task.display();
        }
    }
    void filterTasksByTag(const std::string& tagName) const {
        std::cout << "Tasks with tag '" << tagName << "':\n";
        for (const auto& task : tasks) {
            if (task.hasTag(tagName)) {
                task.display();
            }
        }
    }

    void filterTasksByCategory(Category& category) const {
        std::cout << "Tasks with category '" << category.getName() << "':\n";
        for (const auto& task : tasks) {
            if (category.hasTask(task.getName()) ){
                task.display();
            }
        }
    }
    void sortTasksByDDL() {
        std::sort(tasks.begin(), tasks.end(),
            [](const Task& t1, const Task& t2) { return t1.getDdl() < t2.getDdl(); });
        std::cout << "Tasks sorted by DDL.\n";
    }

    void sortTasksByPriority() {
        std::sort(tasks.begin(), tasks.end(),
            [](const Task& t1, const Task& t2) { return t1.getPriority() < t2.getPriority(); });
        std::cout << "Tasks sorted by priority.\n";
    }
    void batchAddTags(const std::vector<std::string>& taskNames, const std::string& tagName) {
        BatchOperation::addTagsToTasks(tasks, taskNames, tagName);
    }

    void batchCategorizeTasks(const std::vector<std::string>& taskNames, Category& category) {
        BatchOperation::categorizeTasks(tasks, taskNames, category);
    }

    void batchDeleteTasks(const std::vector<std::string>& taskNames) {
        BatchOperation::deleteTasks(tasks, taskNames);
    }

    const std::vector<Task>& getTasks() const {
        return tasks;
    }
};
void saveTasksToFile(const std::vector<Task>& tasks, const std::string& filename) {
    std::ofstream file(filename, std::ios::out);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for saving tasks.\n";
        return;
    }

    for (const auto& task : tasks) {
        file << task.getName() << "|"
            << task.getDescription() << "|"
            << task.getDdl() << "|"
            << task.getPriority() << "|"
            << task.getReminder().getRemindTime() << "\n";
    }

    file.close();
    std::cout << "Tasks saved to file successfully.\n";
}

// 从文件加载任务
void loadTasksFromFile(TaskManager& manager, const std::string& filename) {
    std::ifstream file(filename, std::ios::in);
    if (!file.is_open()) {
        std::cerr << "No existing task file found. Starting fresh.\n";
        return;
    }

    time_t now = time(nullptr);
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string name, description, ddlStr, remindTimeStr, priorityStr;

        std::getline(ss, name, '|');
        std::getline(ss, description, '|');
        std::getline(ss, ddlStr, '|');
        std::getline(ss, priorityStr, '|');
        std::getline(ss, remindTimeStr);

        time_t ddl = std::stol(ddlStr);
        time_t remindTime = std::stol(remindTimeStr);
        int priority = std::stoi(priorityStr);

        if (ddl < now) {
            std::cout << "Skipping expired task: " << name << "\n";
            continue;
        }

        Reminder reminder(remindTime, "Notification");
        manager.createTask(name, description, ddl, priority, reminder);
    }

    file.close();
    std::cout << "Tasks loaded from file successfully.\n";
}

// 创建任务的函数
void createTask(TaskManager& manager) {
    system("cls");
    std::string name, description, ddlStr, remindOption;
    int priority;

    std::cin.ignore();
    std::cout << "Enter task name: ";
    std::getline(std::cin, name);
    std::cout << "Enter description: ";
    std::getline(std::cin, description);
    std::cout << "Enter DDL (YYYY-MM-DD HH:MM:SS): ";
    std::getline(std::cin, ddlStr);
    std::cout << "Enter priority: ";
    std::cin >> priority;

    time_t ddl = parseDatetime(ddlStr);
    if (ddl == -1) return;

    std::cout << "Select reminder time: (1) 5 mins (2) 1 hour (3) 1 day: ";
    std::cin >> remindOption;

    time_t reminderTime = ddl;
    if (remindOption == "1") reminderTime -= 300;
    else if (remindOption == "2") reminderTime -= 3600;
    else if (remindOption == "3") reminderTime -= 86400;

    Reminder reminder(reminderTime, "Notification");
    manager.createTask(name, description, ddl, priority, reminder);
}

// 查看任务的函数
void listTasks(TaskManager& manager) {
    system("cls");
    std::cout << "Would you like to filter tasks? (y/n): ";
    char filterChoice;
    std::cin >> filterChoice;

    if (filterChoice == 'y') {
        std::cin.ignore();
        std::cout << "Enter tag to filter by: ";
        std::string tag;
        std::getline(std::cin, tag);
        manager.filterTasksByTag(tag);
    }

    system("cls");
    std::cout << "Sort tasks by: (1) DDL (2) Priority: ";
    int sortChoice;
    std::cin >> sortChoice;

    if (sortChoice == 1) {
        manager.sortTasksByDDL();
    }
    else if (sortChoice == 2) {
        manager.sortTasksByPriority();
    }

    manager.listTasks();

    std::cout << "Return to main menu? (y/n): ";
    char returnChoice;
    std::cin >> returnChoice;
    if (returnChoice != 'y') return;
}

// 批量操作的函数
void batchOperation(TaskManager& manager) {
    system("cls");
    std::cout << "Batch Operation:\n"
        << "1. Add Tags\n"
        << "2. Categorize Tasks\n"
        << "3. Delete Tasks\n"
        << "Enter your choice: ";
    int batchChoice;
    std::cin >> batchChoice;

    std::cin.ignore();
    std::vector<std::string> taskNames;
    std::string names;
    std::cout << "Enter task names (comma separated): ";
    std::getline(std::cin, names);
    std::stringstream ss(names);
    while (std::getline(ss, names, ',')) {
        taskNames.push_back(names);
    }

    if (batchChoice == 1) {
        std::cout << "Enter tag to add: ";
        std::string tag;
        std::cin >> tag;
        manager.batchAddTags(taskNames, tag);
    }
    else if (batchChoice == 2) {
        std::cout << "Enter category name: ";
        std::string categoryName;
        std::cin >> categoryName;
        Category category(categoryName);
        manager.batchCategorizeTasks(taskNames, category);
    }
    else if (batchChoice == 3) {
        manager.batchDeleteTasks(taskNames);
    }
}

// 显示主菜单
int displayMainMenu() {
    system("cls");
    time_t now = time(nullptr);
    std::cout << "The time now is : " << formatDatetime(now) << std::endl;
    std::cout << "\nMain Menu:\n"
        << "1. Create Task\n"
        << "2. List Task\n"
        << "3. Batch Operation\n"
        << "4. Exit\n"
        << "Enter your choice: ";
    int choice;
    std::cin >> choice;
    return choice;
}

// 程序退出前的保存任务
void exitProgram(TaskManager& manager, const std::string& filename) {
    system("cls");
    saveTasksToFile(manager.getTasks(), filename);
    std::cout << "Exiting program. Goodbye!\n";
}

// 主程序逻辑
void runTaskManager() {
    TaskManager manager;
    const std::string filename = "tasks.txt";
    loadTasksFromFile(manager, filename);

    // 开启提醒线程
    std::thread reminderThread([&manager]() {
        runReminder(manager.getTasks());
        });
    reminderThread.detach();

    while (true) {
        int choice = displayMainMenu();

        if (choice == 1) {
            createTask(manager);
        }
        else if (choice == 2) {
            listTasks(manager);
        }
        else if (choice == 3) {
            batchOperation(manager);
        }
        else if (choice == 4) {
            exitProgram(manager, filename);
            break;
        }
    }
}

int main() {
    runTaskManager();
    return 0;
}
