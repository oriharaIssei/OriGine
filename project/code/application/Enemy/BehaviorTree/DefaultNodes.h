#pragma once

///stl
//memory
#include <memory>
//container
#include <vector>

class IEnemy;
///===============================================================
/// BehaviorTree の 各ノードを表す クラス群
///===============================================================
namespace EnemyBehavior {
enum class NodeType {
    Action,    // 1 つの 処理
    Condition, // boolで表せるもの
    Selector,  // 実行するNode を選択するもの
    Decorator,
    Sequence // すべてが 成功するか，何かが失敗するまで実行するもの
};
enum class Status {
    FAILURE,
    RUNNING,
    SUCCESS
};

class Node {
public:
    Node(NodeType _type)
        : type_(_type) {}
    virtual ~Node() = default;

    virtual Status tick() = 0;

protected:
    IEnemy* enemy_ = nullptr;

private:
    NodeType type_;

public:
    NodeType getType() const { return type_; }

    void setEnemy(IEnemy* _enemy) { enemy_ = _enemy; }
};

class Action
    : public Node {
public:
    Action()
        : Node(NodeType::Action) {}
    ~Action(){}
    virtual Status tick() = 0;
};

class Condition
    : public Node {
public:
    Condition()
        : Node(NodeType::Condition) {}
    ~Condition() {}
    virtual Status tick() = 0;
};

class Sequence
    : public Node {
public:
    Sequence()
        : Node(NodeType::Sequence) {}
    ~Sequence() {}

    Status tick() override {
        while (currentChild < children.size()) {
            Status status = children[currentChild]->tick();
            if (status == Status::FAILURE) {
                currentChild = 0;
                return Status::FAILURE;
            }
            /*if(status == Status::RUNNING){
					return Status::RUNNING;
				}*/
            ++currentChild;
        }
        currentChild = 0;
        return Status::SUCCESS;
    }

private:
    std::vector<std::unique_ptr<Node>> children;
    size_t currentChild = 0;

public:
    void addChild(std::unique_ptr<Node> child) {
        children.push_back(std::move(child));
    }
};

class Selector
    : public Node {
public:
    Selector()
        : Node(NodeType::Selector) {}
    ~Selector() {}

    Status tick() override {
        while (currentChild < children.size()) {
            Status status = children[currentChild]->tick();
            if (status == Status::SUCCESS) {
                currentChild = 0;
                return Status::SUCCESS;
            }
            /*if(status == Status::RUNNING){
						return Status::RUNNING;
				}*/
            ++currentChild;
        }
        currentChild = 0;
        return Status::FAILURE;
    }

private:
    std::vector<std::unique_ptr<Node>> children;
    size_t currentChild = 0;

public:
    void addChild(std::unique_ptr<Node> child) {
        children.push_back(std::move(child));
    }
};
} // namespace EnemyBehavior
