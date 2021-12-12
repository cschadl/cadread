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

void cadread_ConsoleProgressIndicator::Show(
    Message_ProgressScope const& scope,
    const Standard_Boolean force /* = Standard_False */)
{
	double const pc = GetPosition() * 100;

	printf("%s : %.*f%%\n\033[F\033[J", scope.Name(), precision(), pc);
}
