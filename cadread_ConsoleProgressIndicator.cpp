#include "cadread_ConsoleProgressIndicator.h"

#include <iomanip>
#include <iostream>
#include <string>

using namespace std;

cadread_ConsoleProgressIndicator::cadread_ConsoleProgressIndicator(size_t precision)
: Message_ProgressIndicator()
, m_precision(precision)
{

}

Standard_Boolean cadread_ConsoleProgressIndicator::Show(const Standard_Boolean force /* = Standard_False */)
{
	double const pc = GetPosition() * 100;

	Standard_Integer n_scopes = GetNbScopes();

	cout << string(GetScope(n_scopes).GetName()->ToCString())
		 << ": " << fixed << setprecision(precision()) << pc << "%" << endl;

	return Standard_True;
}
