#ifndef _CADREAD_CONSOLE_PROGRESSINDICATOR_H
#define _CADREAD_CONSOLE_PROGRESSINDICATOR_H

#include <Message_ProgressIndicator.hxx>

class cadread_ConsoleProgressIndicator : public Message_ProgressIndicator
{
private:
	int m_precision;

public:
	cadread_ConsoleProgressIndicator(int precision);

	int precision() const { return m_precision; }

	Standard_Boolean Show(const Standard_Boolean force = Standard_True) override;
};

#endif
