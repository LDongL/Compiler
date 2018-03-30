#include "parsers.h"
#include "syntax_descriptors.h"

using namespace compiler::syntax;

compiler::parser_if::parser_if(syntax_analyzer * analyzer)
	: parser(analyzer)
{
}

compiler::AST compiler::parser_if::parse() {
	if (!m_syntax_analyzer->peek_token("if"))
		return nullptr;

	AST ast = make_shared<syntax_tree>(descriptor_if(), m_syntax_analyzer->peek_token_with_code_info());

	m_syntax_analyzer->assert_next_token("if");
	m_syntax_analyzer->assert_next_token("(");
	ast->children.push_back(m_syntax_analyzer->parse_expression());
	m_syntax_analyzer->assert_next_token(")");
	ast->children.push_back(m_syntax_analyzer->parse_packed_statements());
	if (m_syntax_analyzer->peek_token("else")) {
		m_syntax_analyzer->assert_next_token("else");
		ast->children.push_back(m_syntax_analyzer->parse_packed_statements());
	}

	return ast;
}

compiler::AST compiler::parser_for::parse_for1() {
	if (is_name(m_syntax_analyzer->peek_token(0)) && is_name(m_syntax_analyzer->peek_token(1)))
	{
		return m_syntax_analyzer->parse_define_vars();
	}
	else
	{
		AST ast = m_syntax_analyzer->parse_expression();
		m_syntax_analyzer->assert_next_token(";");
		return ast;
	}
}

compiler::parser_for::parser_for(syntax_analyzer * analyzer)
	: parser(analyzer)
{
}

compiler::AST compiler::parser_for::parse() {
	if (!m_syntax_analyzer->peek_token("for"))
		return nullptr;

	AST ast = make_shared<syntax_tree>(descriptor_statements(), m_syntax_analyzer->peek_token_with_code_info());

	m_syntax_analyzer->assert_next_token("for");
	m_syntax_analyzer->assert_next_token("(");
	if (!m_syntax_analyzer->peek_token(";"))
		ast->children.push_back(parse_for1());

	AST nest = make_shared<syntax_tree>(descriptor_while(), m_syntax_analyzer->peek_token_with_code_info());
	if (m_syntax_analyzer->peek_token(";"))
		nest->children.push_back(make_shared<syntax_tree>(descriptor_constant("true"), m_syntax_analyzer->peek_token_with_code_info()));
	else
		nest->children.push_back(m_syntax_analyzer->parse_expression());

	m_syntax_analyzer->assert_next_token(";");

	AST f = nullptr;
	if (!m_syntax_analyzer->peek_token(")"))
		f = m_syntax_analyzer->parse_expression();
	m_syntax_analyzer->assert_next_token(")");

	AST statements = make_shared<syntax_tree>(descriptor_statements(), m_syntax_analyzer->peek_token_with_code_info());
	statements->add_children(m_syntax_analyzer->parse_packed_statements());
	if (f != nullptr)
		statements->add_children(f);

	nest->add_children(statements);
	ast->add_children(nest);

	return ast;
}

compiler::parser_while::parser_while(syntax_analyzer * analyzer)
	: parser(analyzer)
{
}

compiler::AST compiler::parser_while::parse()
{
	if (!m_syntax_analyzer->peek_token("while"))
		return nullptr;

	AST ast = make_shared<syntax_tree>(descriptor_while(), m_syntax_analyzer->peek_token_with_code_info());

	m_syntax_analyzer->assert_next_token("while");
	m_syntax_analyzer->assert_next_token("(");
	ast->children.push_back(m_syntax_analyzer->parse_expression());
	m_syntax_analyzer->assert_next_token(")");
	ast->children.push_back(m_syntax_analyzer->parse_packed_statements());

	return ast;
}

compiler::parser_return::parser_return(syntax_analyzer * analyzer)
	: parser(analyzer)
{
}

compiler::AST compiler::parser_return::parse()
{
	if (!m_syntax_analyzer->peek_token("return"))
		return nullptr;

	AST ast = make_shared<syntax_tree>(descriptor_return(), m_syntax_analyzer->peek_token_with_code_info());

	m_syntax_analyzer->assert_next_token("return");
	ast->children.push_back(m_syntax_analyzer->parse_expression());
	m_syntax_analyzer->assert_next_token(";");

	return ast;
}