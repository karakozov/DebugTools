#ifndef REG_TEST_H
#define REG_TEST_H

#include <QtWidgets/QDialog>
#include "ui_reg_test.h"

class reg_test : public QDialog, private Ui::reg_testClass
{
	Q_OBJECT

public:
	reg_test(QWidget *parent = 0);
	~reg_test();

public slots:
	void updateDeviceInfo();
	void updateFifoInfo();
	void ClickedRead();
	void ClickedWrite();
	void ClickedReadSpd();
	void ClickedWriteSpd();

private:
	void SetDeviceList();
	void SetTetrList();
	void GetStatus(int iTetr);
	void FifoInfo(int iTetr);

protected:
	void keyPressEvent(QKeyEvent *pEvent);
};

#endif // REG_TEST_H
