#pragma once

#include <memory>
#include <vector>

///===============================================================
/// BehaviorTree の 各ノードを表す クラス群
///===============================================================
namespace MyTree{
	enum class NodeType{
		Selector,
		Sequence,
		Action,
		Condition,
		Decorator
	};
	enum class Status{
		SUCCESS,
		FAILURE
		//RUNNING
	};

	class Node{
	public:
		Node(NodeType _type):type_(_type){}
		~Node() = default;

		virtual Status tick() = 0;
	private:
		NodeType type_;
	public:
		NodeType getType()const{ return type_; }
	};

	class Action : public Node{
	public:
		Action():Node(NodeType::Action){}
		virtual Status tick() = 0;
	};

	class Condition : public Node{
	public:
		Condition():Node(NodeType::Condition){}
		virtual Status tick() = 0;
	};

	class Sequence : public Node{
	public:
		Sequence():Node(NodeType::Sequence){}

		Status tick() override{
			while(currentChild < children.size()){
				Status status = children[currentChild]->tick();
				if(status == Status::FAILURE){
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
		void addChild(std::unique_ptr<Node> child){
			children.push_back(std::move(child));
		}
	};

	class Selector : public Node{
	public:
		Selector():Node(NodeType::Selector){}
		Status tick() override{
			while(currentChild < children.size()){
				Status status = children[currentChild]->tick();
				if(status == Status::SUCCESS){
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
		void addChild(std::unique_ptr<Node> child){
			children.push_back(std::move(child));
		}
	};
}