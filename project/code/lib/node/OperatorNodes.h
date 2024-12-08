#pragma once

#include <memory>
#include <type_traits>

#include "DefaultNodes.h"

namespace MyTree{
#pragma region"Concepts"
	// 加算が可能な型
	template <typename T,typename U,typename H>
	concept Addable = requires(T t,U u){
		{ t + u } -> std::convertible_to<H>;
	};

	// 減算が可能な型
	template <typename T,typename U,typename H>
	concept Subtractable = requires(T t,U u){
		{ t - u } -> std::convertible_to<H>;
	};

	// 乗算が可能な型
	template <typename T,typename U,typename H>
	concept Multipliable = requires(T t,U u){
		{ t* u } -> std::convertible_to<H>;
	};

	// 除算が可能な型
	template <typename T,typename U,typename H>
	concept Divisible = requires(T t,U u){
		{ t / u } -> std::convertible_to<H>;
	};

	// 代入が可能な型
	template <typename T,typename U,typename H>
	concept assignable  = requires(T t,U u){
		{ t = u } -> std::convertible_to<H>;
	};
#pragma endregion

	/// <summary>
	/// 計算 処理 ノード
	/// </summary>
	/// <typeparam name="Left">左辺値</typeparam>
	/// <typeparam name="Right">右辺値</typeparam>
	/// <typeparam name="Return">回答値</typeparam>
	template <typename Left,typename Right,typename Return>
	class OperatorNode
		:Action{
	public:
		OperatorNode(ValueNode<Left>* _left,ValueNode<Left>* _right)
			:Action(),left_(_left),
			right_(_right),
			return_(Return()),
			returnNode_(std::make_unique<ValueNode<Return>>(&return_)){}
		~OperatorNode(){}

		virtual Status tick() = 0;
	protected:
		ValueNode<Left>* left_;
		ValueNode<Right>* right_;
		Return return_;
		std::unique_ptr<ValueNode<Return>> returnNode_;
	public:
		const Left& getLeft()const{ return left_; }
		const Right& getRight()const{ return right_; }
		const Return& getReturn()const{ return return_; }

		ValueNode<Return>* getReturnNode()const{ return returnNode_.get(); }
	};

	/// <summary>
	/// 加算処理ノード
	/// </summary>
	/// <typeparam name="Left">左辺値</typeparam>
	/// <typeparam name="Right">右辺値</typeparam>
	/// <typeparam name="Return">回答値</typeparam>
	template <typename Left,typename Right,typename Return>
		requires Addable<Left,Right,Return>
	class AddNode :
		public OperatorNode<Left,Right,Return>{
	public:
		AddNode(const Left& _left,const Right& _right)
			: OperatorNode<Left,Right,Return>(_left,_right){}
		~AddNode(){}

		Status tick(){
			this->return_ = this->left_ + this->right_;
			return Status::SUCCESS;
		}
	};

	/// <summary>
	/// 減算処理ノード
	/// </summary>
	/// <typeparam name="Left">左辺値</typeparam>
	/// <typeparam name="Right">右辺値</typeparam>
	/// <typeparam name="Return">回答値</typeparam>
	template <typename Left,typename Right,typename Return>
		requires Subtractable<Left,Right,Return>
	class SubtractNode :
		public OperatorNode<Left,Right,Return>{
	public:
		SubtractNode(const Left& _left,const Right& _right)
			: OperatorNode<Left,Right,Return>(_left,_right){}
		~SubtractNode(){}

		Status tick(){
			this->return_ = this->left_ - this->right_;
			return Status::SUCCESS;
		}
	};

	/// <summary>
	/// 乗算処理ノード
	/// </summary>
	/// <typeparam name="Left">左辺値</typeparam>
	/// <typeparam name="Right">右辺値</typeparam>
	/// <typeparam name="Return">回答値</typeparam>
	template <typename Left,typename Right,typename Return>
		requires Multipliable<Left,Right,Return>
	class MultiplyNode :
		public OperatorNode<Left,Right,Return>{
	public:
		MultiplyNode(const Left& _left,const Right& _right)
			: OperatorNode<Left,Right,Return>(_left,_right){}
		~MultiplyNode(){}

		Status tick(){
			this->return_ = this->left_ * this->right_;
			return Status::SUCCESS;
		}
	};

	/// <summary>
	/// 除算処理ノード
	/// </summary>
	/// <typeparam name="Left">左辺値</typeparam>
	/// <typeparam name="Right">右辺値</typeparam>
	/// <typeparam name="Return">回答値</typeparam>
	template <typename Left,typename Right,typename Return>
		requires Divisible<Left,Right,Return>
	class DivisionNode :
		public OperatorNode<Left,Right,Return>{
	public:
		DivisionNode(const Left& _left,const Right& _right)
			: OperatorNode<Left,Right,Return>(_left,_right){}
		~DivisionNode(){}

		Status tick(){
			this->return_ = this->left_ / this->right_;
			return Status::SUCCESS;
		}
	};

	template <typename Left,typename Right,typename Return>
		requires assignable<Left,Right,Return>
	class AssignmentNode
		:public OperatorNode<Left,Right,Return>{
		AssignmentNode(const Left& _left,const Right& _right)
			: OperatorNode<Left,Right,Return>(_left,_right){}
		~AssignmentNode(){}

		Status tick(){
			this->right_->tick();

			this->left_->SetValue(this->right_);  // 代入操作
			this->return_ = this->left_;  // 結果を左辺に設定
			return Action::Status::SUCCESS;
		}
	};
}