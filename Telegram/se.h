#pragma once
class ApiService: public QThread
{
	Q_OBJECT;
	void run() override;

public:
	ApiService();
	~ApiService();
};

