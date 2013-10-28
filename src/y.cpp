#include <QDir>
#include <QFile>
#include <ring/ring.hpp>
#include <prng/prng.hpp>
#include <aes/aes.hpp>

bool encodeDir(QDir& dir, QFile& log)//, Ring& ring, Aes& aes)
{
	dir.setFilter(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
	QList<QFileInfo> files = dir.entryInfoList();
	QFileInfo logInfo(log);
	QString logPath = logInfo.absolutePath()+QDir::separator();
	for (int i=0; i<files.length(); i++)
	{
		QFileInfo file = files.at(i);
		Prng prng;
		if (file.isFile() && file.isReadable())
		{
			char key[64];
			prng.generate(key, 64);
			QString filePath = file.absoluteFilePath();
			unsigned int length = filePath.length();
			for (int i=0; i<logPath.length(); i++)
			{
				if (logPath.at(i) == filePath.at(i))
				{
					length--;
				}
				else
				{
					break;
				}
			}
			filePath = filePath.right(length);
			unsigned int filePathLength = filePath.length();
			unsigned int filePathLengthHigh = (filePathLength>>8) & 0xff;
			unsigned int filePathLengthLow = filePathLength & 0xff;
			char filePathLengthString[2];
			filePathLengthString[0] = filePathLengthHigh;
			filePathLengthString[1] = filePathLengthLow;
//TODO encode data
			if (log.write(filePathLengthString, 2) != 2)
			{
				return false;				
			}
			if (log.write(filePath.toStdString().c_str(), filePathLength) != filePathLength)
			{
				return false;				
			}
			if (log.write(key, 64) != 64)
			{
				return false;				
			}
			/*if (!encodeFile(file.absoluteFilePath().toStdString().c_str(), key, 64))
			{
				return false;
			}*/
		}
		else if (file.isDir() && file.isReadable())
		{
			QDir dir(file.absoluteFilePath());
			if (!encodeDir(dir, log))//, ring, aes);
			{
				return false;
			}
		}
	}
	return true;
}

bool decodeDir(QFile& log)//, Ring& ring, Aes& aes)
{
	QFileInfo logInfo(log);
	QString logPath = logInfo.absolutePath()+QDir::separator();
	unsigned int treated = 0;
	unsigned int fileSize = log.size();
		char filePathLengthString[2];
		char key[64];
		//char filePath[filePathLength+1];
		char filePath[512];
	while (treated < fileSize)
	{
		if (log.read(filePathLengthString, 2) != 2)
		{
			return false;
		}
		unsigned int filePathLengthHigh = filePathLengthString[0];
		unsigned int filePathLengthLow = filePathLengthString[1];
		unsigned int filePathLength = (filePathLengthHigh<<8) | filePathLengthLow;
printf(".\n");
printf(".\n");
		if (log.read(filePath, filePathLength) != filePathLength)
		{
			return false;
		}
		filePath[filePathLength] = '\0';
		if (log.read(key, 64) != 64)
		{
			return false;
		}
		QString qFilePath(logPath);
		qFilePath.append(filePath);
printf("%s %u %u %u\n", qFilePath.toStdString().c_str(), filePathLength, treated, fileSize);
		/*if (!decodeFile())
		{
			return false;
		}*/
		treated += 2+filePathLength+64;
	}
	return true;
}

int main()
{
	QDir home = QDir::home();
	QFile file("keyfile");
	if (file.exists())
	{
		printf("ERROR: keyfile already exists\n");
		return -1;
	}
	if (!file.open(QIODevice::WriteOnly))
	{
		printf("ERROR: keyfile not openable\n");
		return -1;
	}
	if (!encodeDir(home, file))
	{
		return -1;
	}
	file.close();

	if (!file.open(QIODevice::ReadOnly))
	{
		printf("ERROR: keyfile not openable\n");
		return -1;
	}
	if (!decodeDir(file))
	{
		return -1;
	}
	file.close();
	file.remove();
	return 0;
}
