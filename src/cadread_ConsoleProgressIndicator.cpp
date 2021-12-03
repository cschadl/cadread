#include "cadread_ConsoleProgressIndicator.h"

#include <iomanip>
#include <iostream>
#include <string>
#include <cstdio>

using namespace std;

cadread_ConsoleProgressIndicator::cadread_ConsoleProgressIndicator(int precision)
: Message_ProgressIndicator()
, m_precision(precision)
{

}

Standard_Boolean cadread_ConsoleProgressIndicator::Show(const Standard_Boolean force /* = Standard_False */)
{
	double const pc = GetPosition() * 100;
	Standard_Integer const n_scopes = GetNbScopes();

	printf("%s : %.*f%%\n\033[F\033[J", GetScope(n_scopes).GetName()->ToCString(), precision(), pc);

	return Standard_True;
}
