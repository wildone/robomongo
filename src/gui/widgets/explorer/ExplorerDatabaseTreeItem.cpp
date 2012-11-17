#include "StdAfx.h"
#include "ExplorerDatabaseTreeItem.h"
#include "ExplorerDatabaseViewModel.h"
#include "AppRegistry.h"
#include "ExplorerCollectionTreeItem.h"
#include "ExplorerCollectionViewModel.h"
#include "ExplorerDatabaseCategoryTreeItem.h"

/*
** Constructs DatabaseTreeItem
*/
ExplorerDatabaseTreeItem::ExplorerDatabaseTreeItem(ExplorerDatabaseViewModel * viewModel) : QObject()
{
	_viewModel = viewModel;

	setText(0, _viewModel->databaseName());;
	setIcon(0, AppRegistry::instance().databaseIcon());
	setExpanded(true);

	setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);

	connect(_viewModel, SIGNAL(collectionsRefreshed()), SLOT(vm_collectionRefreshed()));


	QIcon icon = AppRegistry::instance().folderIcon();

	_collectionItem = new ExplorerDatabaseCategoryTreeItem(Collections, this);
	_collectionItem->setText(0, "Collections");
	_collectionItem->setIcon(0, icon);
	_collectionItem->setExpanded(true);
	_collectionItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
	addChild(_collectionItem);

	_javascriptItem = new ExplorerDatabaseCategoryTreeItem(Functions, this);
	_javascriptItem->setText(0, "Functions");
	_javascriptItem->setIcon(0, icon);
	_javascriptItem->setExpanded(true);
	_javascriptItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
	addChild(_javascriptItem);

	_filesItem = new ExplorerDatabaseCategoryTreeItem(Files, this);
	_filesItem->setText(0, "Files");
	_filesItem->setIcon(0, icon);
	_filesItem->setExpanded(true);
	_filesItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
	addChild(_filesItem);

	_usersItem = new ExplorerDatabaseCategoryTreeItem(Users, this);
	_usersItem->setText(0, "Users");
	_usersItem->setIcon(0, icon);
	_usersItem->setExpanded(true);
	_usersItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
	addChild(_usersItem);
}

/*
** Expand database tree item;
*/
void ExplorerDatabaseTreeItem::expandCollections()
{
	_viewModel->expandCollections();
}

void ExplorerDatabaseTreeItem::vm_collectionRefreshed()
{
	// remove child items
	int itemCount = _collectionItem->childCount();
	for (int i = 0; i < itemCount; ++i)
	{
		QTreeWidgetItem * p = _collectionItem->child(0);
		_collectionItem->removeChild(p);
		delete p;
	}

    // Add system folder
    QIcon folderIcon = AppRegistry::instance().folderIcon();
    QTreeWidgetItem * systemFolder = new QTreeWidgetItem();
    systemFolder->setIcon(0, folderIcon);
    systemFolder->setText(0, "System");
    _collectionItem->addChild(systemFolder);

	foreach(ExplorerCollectionViewModel * collection, _viewModel->collections())
	{
        if (collection->system())
        {
            ExplorerCollectionTreeItem * collectionItem = new ExplorerCollectionTreeItem(collection);
            systemFolder->addChild(collectionItem);
            continue;
        }

		ExplorerCollectionTreeItem * collectionItem = new ExplorerCollectionTreeItem(collection);
		_collectionItem->addChild(collectionItem);
	}	
}
