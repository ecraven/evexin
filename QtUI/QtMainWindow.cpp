/*
 * Copyright (c) 2013-2014 Kevin Smith
 * Licensed under the GNU General Public License v3.
 * See Documentation/Licenses/GPLv3.txt for more information.
 */

#include <Eve-Xin/QtUI/QtMainWindow.h>

#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

#include <QAction>
#include <QBoxLayout>
#include <QDateTime>
#include <QImage>
#include <QMenu>
#include <QMenuBar>
#include <QPixmap>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTabWidget>
#include <QTreeView>

#include <Swiften/Base/foreach.h>

#include <Swift/QtUI/QtSwiftUtil.h>

#include <Eve-Xin/Controllers/Character.h>
#include <Eve-Xin/Controllers/DataController.h>

#include <Eve-Xin/QtUI/QtAPIKeyWindow.h>
#include <Eve-Xin/QtUI/QtCharacterList.h>
#include <Eve-Xin/QtUI/QtCharacterPane.h>
#include <Eve-Xin/QtUI/QtSkillDelegate.h>
#include <Eve-Xin/QtUI/QtSkillModel.h>
#include <Eve-Xin/QtUI/QtSkillPlannerWidget.h>

namespace EveXin {

QtMainWindow::QtMainWindow(boost::shared_ptr<DataController> dataController, bool debug) {
	dataController_ = dataController;
	apiWindow_ = NULL;
	QWidget* mainWidget = new QWidget();
	setCentralWidget(mainWidget); // Takes ownership
	QBoxLayout* rightLeftLayout = new QBoxLayout(QBoxLayout::LeftToRight, mainWidget);

	characterList_ = new QtCharacterList(this);
	rightLeftLayout->addWidget(characterList_);

	QTabWidget* tabs = new QTabWidget(this);
	rightLeftLayout->addWidget(tabs);

	QWidget* characterTab = new QWidget(this);
	tabs->addTab(characterTab, "Character Sheet");
	QBoxLayout* characterLayout = new QBoxLayout(QBoxLayout::LeftToRight);
	characterTab->setLayout(characterLayout);
	
	characterPane_ = new QtCharacterPane(this);
	characterLayout->addWidget(characterPane_);

	QBoxLayout* skillsLayout = new QBoxLayout(QBoxLayout::TopToBottom);
	characterLayout->addLayout(skillsLayout);

	trainingPane_ = new QTreeView(this);
	trainingModel_ = boost::make_shared<QtSkillModel>();
	trainingPane_->setModel(trainingModel_.get());
	trainingPane_->setUniformRowHeights(false);
	trainingPane_->setHeaderHidden(true);
	QtSkillDelegate* delegate = new QtSkillDelegate(this);
	trainingPane_->setItemDelegate(delegate);
	skillsLayout->addWidget(trainingPane_);

	skillPane_ = new QTreeView(this);
	skillModel_ = boost::make_shared<QtSkillModel>();
	skillPane_->setModel(skillModel_.get());
	skillPane_->setUniformRowHeights(false);
	skillPane_->setHeaderHidden(true);
	delegate = new QtSkillDelegate(this);
	skillPane_->setItemDelegate(delegate);
	skillsLayout->addWidget(skillPane_);

	skillPlannerWidget_ = new QtSkillPlannerWidget(dataController, this);
	tabs->addTab(skillPlannerWidget_, "Skill Plan");

	if (debug) {
		debugConsole_ = new QPlainTextEdit(this);
		debugConsole_->setReadOnly(true);
		tabs->addTab(debugConsole_, "Debug");
	}

	characterList_->onCharacterSelected.connect(boost::bind(&QtMainWindow::handleCharacterSelected, this, _1));
	handleCharacterListUpdated();
	dataController_->onCharacterListChanged.connect(boost::bind(&QtMainWindow::handleCharacterListUpdated, this));
	dataController_->onDebugMessage.connect(boost::bind(&QtMainWindow::handleDebugData, this, _1));
	createMenus();
}

QtMainWindow::~QtMainWindow() {
	if (currentCharacter_) {
		currentCharacter_->onDataChanged.disconnect(boost::bind(&QtMainWindow::handleCharacterDataChanged, this));
	}
	delete apiWindow_;
}

void QtMainWindow::createMenus() {
	QMenu* menu = menuBar()->addMenu("General");
	QAction* editAPIKeyAction = menu->addAction("Edit API Keys");
	connect(editAPIKeyAction, SIGNAL(triggered()), this, SLOT(handleEditAPIKeys()));
}

void QtMainWindow::handleEditAPIKeys() {
	if (!apiWindow_) {
		apiWindow_ = new QtAPIKeyWindow(dataController_);
	}
	apiWindow_->show();
}

void QtMainWindow::handleCharacterListUpdated() {
	auto characterIDs = dataController_->getCharacters();
	std::vector<Character::ref> characters;
	foreach (const std::string id, characterIDs) {
		characters.push_back(dataController_->getCharacter(id));
	}
	std::sort(characters.begin(), characters.end(), [](Character::ref a, Character::ref b){return a->getName() < b->getName();});
	characterList_->setCharacters(characters);
}

void QtMainWindow::handleCharacterSelected(Character::ref character) {
	if (currentCharacter_) {
		currentCharacter_->onDataChanged.disconnect(boost::bind(&QtMainWindow::handleCharacterDataChanged, this));
	}
	if (!character) {
		return;
	}
	character->update();
	characterPane_->setCharacter(character);
	currentCharacter_ = character;
	handleCharacterDataChanged();
	currentCharacter_->onDataChanged.connect(boost::bind(&QtMainWindow::handleCharacterDataChanged, this));
}

void QtMainWindow::handleCharacterDataChanged() {
	trainingModel_->setRoot(currentCharacter_->getTrainingQueue());
	skillModel_->setRoot(currentCharacter_->getKnownSkills());

	skillPlannerWidget_->setCharacter(currentCharacter_);
	trainingModel_->setCharacter(currentCharacter_);
	trainingPane_->expandAll();

}

void QtMainWindow::handleDebugData(const std::string& data) {
	if (debugConsole_) {
		debugConsole_->appendPlainText("");
		debugConsole_->appendPlainText(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
		debugConsole_->appendPlainText(P2QSTRING(data));
	}
}

}
