#include "instructions.h"
#include "virtual_machine.h"
#include "unit_assert.h"
using namespace compiler;
using namespace compiler::instructions;

compiler::instructions::instruction_jump::instruction_jump(int to) : to(to) {}

void compiler::instructions::instruction_jump::operate(function_environment & env)
{
	env.jump(to);
}

compiler::instructions::instruction_if::instruction_if(int to) : to(to) {}

void compiler::instructions::instruction_if::operate(function_environment & env)
{
	if (!env.stack->pop_value<int>())
	{
		env.jump(to);
	}
}

void compiler::instructions::instruction_return::operate(function_environment & env)
{
	env.mark_return();
}

compiler::instructions::instruction_invoke::instruction_invoke(const string & name, int arg_num) : name(name), arg_num(arg_num) {}

void compiler::instructions::instruction_invoke::operate(function_environment & env)
{
	vector<object_t> args;
	for (int i = 0; i < arg_num; ++i)
		args.push_back(env.stack->pop().value);
	reverse(args.begin(), args.end());

	auto result = env.vm->execute_function(name, args);
	if (!result.empty()) // void
		env.stack->push(operand_stack_element(result));
}

void compiler::instructions::instruction_pop::operate(function_environment & env)
{
	env.stack->pop();
}

void compiler::instructions::instruction_store::operate(function_environment & env)
{
	auto rhs = env.stack->pop();
	auto lhs = env.stack->pop();

	assert_cond(lhs.addr != nullptr);
	memcpy(lhs.addr, rhs.value.get_raw_memory(), rhs.value.get_raw_memory_size());

	operand_stack_element e(rhs.value, lhs.addr);
	env.stack->push(e);
}

void compiler::instructions::instruction_duplicate::operate(function_environment & env)
{
	auto obj = env.stack->pop();
	env.stack->push(obj);
	env.stack->push(obj);
}

instruction_ptr get_instruction(compiler::type_base_ptr type, int index, bool is_static)
{
	if (type->is_array())
		return instruction_ptr(new instruction_load_array(type, index, is_static));
	else if (type->is_pointer())
		return instruction_ptr(new instruction_load<void*>(index, is_static));
	else if (type->is_primitive())
	{
		if (type == type_int)
			return instruction_ptr(new instruction_load<int>(index, is_static));
		else if (type == type_short)
			return instruction_ptr(new instruction_load<short>(index, is_static));
		else if (type == type_long)
			return instruction_ptr(new instruction_load<long>(index, is_static));
		else if (type == type_int64)
			return instruction_ptr(new instruction_load<long long>(index, is_static));
		else if (type == type_float)
			return instruction_ptr(new instruction_load<float>(index, is_static));
		else if (type == type_double)
			return instruction_ptr(new instruction_load<double>(index, is_static));
		else if (type == type_long_double)
			return instruction_ptr(new instruction_load<long double>(index, is_static));
		else if (type == type_char)
			return instruction_ptr(new instruction_load<char>(index, is_static));
		else if (type == type_bool)
			return instruction_ptr(new instruction_load<bool>(index, is_static));
	}
	throw new invalid_argument("Unrecognized type");
}

instruction_ptr compiler::instructions::get_instruction_load(compiler::type_representation type, int index)
{
	return get_instruction(type.base_type, index, type.is_static);
}

compiler::instructions::instruction_reference::instruction_reference(compiler::type_representation type)
	: type(type)
{
}

void compiler::instructions::instruction_reference::operate(function_environment & env)
{
	auto op1 = env.stack->pop();
	auto e = operand_stack_element(op1.addr);
	env.stack->push(e);
}

compiler::instructions::instruction_move::instruction_move(size_t sz, bool move_positive)
	: sz(sz), move_positive(move_positive)
{
}

void compiler::instructions::instruction_move::operate(function_environment & env)
{
	auto op2 = env.stack->pop();
	auto op1 = env.stack->pop();

	int offset = op2.value.cast<int>() * (move_positive ? 1 : -1);

	auto e = operand_stack_element((void*)((char*)op1.value.cast<void*>() + offset * (long long)sz));
	env.stack->push(e);
}

compiler::instructions::instruction_load_array::instruction_load_array(type_base_ptr type, int index, bool is_static)
	: type(type), index(index), is_static(is_static)
{
}

void compiler::instructions::instruction_load_array::operate(function_environment & env)
{
	void* addr;
	if (is_static)
		addr = env.global->get<void*>(index);
	else
		addr = env.local->get<void*>(index);
	operand_stack_element e(addr, addr);
	env.stack->push(e);
}
