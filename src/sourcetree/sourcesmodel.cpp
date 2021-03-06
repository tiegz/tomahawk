/* === This file is part of Tomahawk Player - <http://tomahawk-player.org> ===
 * 
 *   Copyright 2010-2011, Christian Muehlhaeuser <muesli@tomahawk-player.org>
 *
 *   Tomahawk is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Tomahawk is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Tomahawk. If not, see <http://www.gnu.org/licenses/>.
 */

#include "sourcesmodel.h"

#include <QMimeData>
#include <QTimer>
#include <QTreeView>
#include <QStandardItemModel>

#include "tomahawk/tomahawkapp.h"
#include "query.h"
#include "sourcelist.h"
#include "sourcetreeitem.h"
#include "sourcetreeview.h"
#include "utils/imagebutton.h"

using namespace Tomahawk;


SourcesModel::SourcesModel( SourceTreeView* parent )
    : QStandardItemModel( parent )
    , m_parent( parent )
{
    setColumnCount( 1 );

    onSourceAdded( SourceList::instance()->sources() );
    connect( SourceList::instance(), SIGNAL( sourceAdded( Tomahawk::source_ptr ) ), SLOT( onSourceAdded( Tomahawk::source_ptr ) ) );
    connect( SourceList::instance(), SIGNAL( sourceRemoved( Tomahawk::source_ptr ) ), SLOT( onSourceRemoved( Tomahawk::source_ptr ) ) );

    connect( parent, SIGNAL( onOnline( QModelIndex ) ), SLOT( onItemOnline( QModelIndex ) ) );
    connect( parent, SIGNAL( onOffline( QModelIndex ) ), SLOT( onItemOffline( QModelIndex ) ) );
}


QStringList
SourcesModel::mimeTypes() const
{
    QStringList types;
    types << "application/tomahawk.query.list";
    return types;
}


Qt::DropActions
SourcesModel::supportedDropActions() const
{
    return Qt::CopyAction;
}


Qt::ItemFlags
SourcesModel::flags( const QModelIndex& index ) const
{
    Qt::ItemFlags defaultFlags = QStandardItemModel::flags( index );

    if ( index.isValid() )
    {
        if ( indexType( index ) == PlaylistSource )
        {
            playlist_ptr playlist = indexToPlaylist( index );
            if ( !playlist.isNull() && playlist->author()->isLocal() )
                defaultFlags |= Qt::ItemIsEditable;
        }
        else if ( indexType( index ) == DynamicPlaylistSource )
        {
            dynplaylist_ptr playlist = indexToDynamicPlaylist( index );
            if ( !playlist.isNull() && playlist->author()->isLocal() )
                defaultFlags |= Qt::ItemIsEditable;
        }

        return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
    }
    else
        return defaultFlags;
}


QVariant
SourcesModel::data( const QModelIndex& index, int role ) const
{
    if ( role == Qt::SizeHintRole )
    {
        return QSize( 0, 18 );
    }

    return QStandardItemModel::data( index, role );
}


void
SourcesModel::loadSources()
{
    QList<source_ptr> sources = SourceList::instance()->sources();

    foreach( const source_ptr& source, sources )
        appendItem( source );
}


void
SourcesModel::onSourceAdded( const QList<source_ptr>& sources )
{
    foreach( const source_ptr& source, sources )
        appendItem( source );
}


void
SourcesModel::onSourceAdded( const source_ptr& source )
{
    appendItem( source );
}


void
SourcesModel::onSourceRemoved( const source_ptr& source )
{
    removeItem( source );
}


bool
SourcesModel::appendItem( const source_ptr& source )
{
    SourceTreeItem* item = new SourceTreeItem( source, this );
    connect( item, SIGNAL( clicked( QModelIndex ) ), this, SIGNAL( clicked( QModelIndex ) ) );

//    qDebug() << "Appending source item:" << item->source()->username();
    invisibleRootItem()->appendRow( item->columns() );

    if ( !source.isNull() )
    {
        connect( source.data(), SIGNAL( offline() ), SLOT( onSourceChanged() ) );
        connect( source.data(), SIGNAL( online() ), SLOT( onSourceChanged() ) );
        connect( source.data(), SIGNAL( stats( QVariantMap ) ), SLOT( onSourceChanged() ) );
        connect( source.data(), SIGNAL( playbackStarted( Tomahawk::query_ptr ) ), SLOT( onSourceChanged() ) );
        connect( source.data(), SIGNAL( stateChanged() ), SLOT( onSourceChanged() ) );
    }
    
    return true; // FIXME
}


bool
SourcesModel::removeItem( const source_ptr& source )
{
//    qDebug() << "Removing source item from SourceTree:" << source->username();

    QModelIndex idx;
    int rows = rowCount();
    for ( int row = 0; row < rows; row++ )
    {
        QModelIndex idx = index( row, 0 );
        SourceTreeItem* item = indexToTreeItem( idx );
        if ( item )
        {
            if ( item->source() == source )
            {
                qDebug() << "Found removed source item:" << item->source()->userName();
                invisibleRootItem()->removeRow( row );

                onItemOffline( idx );

                delete item;
                return true;
            }
        }
    }

    return false;
}


void
SourcesModel::onItemOnline( const QModelIndex& idx )
{
    qDebug() << Q_FUNC_INFO;

    SourceTreeItem* item = indexToTreeItem( idx );
    if ( item )
        item->onOnline();
}


void
SourcesModel::onItemOffline( const QModelIndex& idx )
{
    qDebug() << Q_FUNC_INFO;

    SourceTreeItem* item = indexToTreeItem( idx );
    if ( item )
        item->onOffline();
}


SourcesModel::SourceType
SourcesModel::indexType( const QModelIndex& index )
{
    if ( !index.isValid() )
        return Invalid;

    QModelIndex idx = index.model()->index( index.row(), 0, index.parent() );
    return static_cast<SourcesModel::SourceType>( idx.data( SourceTreeItem::Type ).toInt() );
}


playlist_ptr
SourcesModel::indexToPlaylist( const QModelIndex& index )
{
    playlist_ptr res;
    if ( !index.isValid() )
        return res;

    if ( indexType( index ) == PlaylistSource )
    {
        QModelIndex idx = index.model()->index( index.row(), 0, index.parent() );
        qlonglong pptr = idx.data( SourceTreeItem::PlaylistPointer ).toLongLong();
        playlist_ptr* playlist = reinterpret_cast<playlist_ptr*>(pptr);
        if ( playlist )
            return *playlist;
    }

    return res;
}


dynplaylist_ptr
SourcesModel::indexToDynamicPlaylist( const QModelIndex& index )
{
    dynplaylist_ptr res;
    if ( !index.isValid() )
        return res;
    
    if ( indexType( index ) == DynamicPlaylistSource )
    {
        QModelIndex idx = index.model()->index( index.row(), 0, index.parent() );
        qlonglong pptr = idx.data( SourceTreeItem::DynamicPlaylistPointer ).toLongLong();
        dynplaylist_ptr* playlist = reinterpret_cast<dynplaylist_ptr*>(pptr);
        if ( playlist )
            return *playlist;
    }
    
    return res;
}


SourceTreeItem*
SourcesModel::indexToTreeItem( const QModelIndex& index )
{
    if ( !index.isValid() )
        return 0;

    int type = indexType( index );
    if ( type == CollectionSource || type == PlaylistSource || type == DynamicPlaylistSource )
    {
        QModelIndex idx = index.model()->index( index.row(), 0, index.parent() );
        qlonglong pptr = idx.data( SourceTreeItem::SourceItemPointer ).toLongLong();
        SourceTreeItem* item = reinterpret_cast<SourceTreeItem*>(pptr);
        if ( item )
            return item;
    }

    return 0;
}


QModelIndex
SourcesModel::playlistToIndex( const Tomahawk::playlist_ptr& playlist )
{
    for ( int i = 0; i < rowCount(); i++ )
    {
        QModelIndex pidx = index( i, 0 );

        for ( int j = 0; j < rowCount( pidx ); j++ )
        {
            QModelIndex idx = index( j, 0, pidx );
            SourcesModel::SourceType type = SourcesModel::indexType( idx );

            if ( type == SourcesModel::PlaylistSource )
            {
                playlist_ptr p = SourcesModel::indexToPlaylist( idx );
                if ( playlist.data() == p.data() )
                    return idx;
            }
        }
    }

    return QModelIndex();
}


QModelIndex
SourcesModel::dynamicPlaylistToIndex( const Tomahawk::dynplaylist_ptr& playlist )
{
    for ( int i = 0; i < rowCount(); i++ )
    {
        QModelIndex pidx = index( i, 0 );
        
        for ( int j = 0; j < rowCount( pidx ); j++ )
        {
            QModelIndex idx = index( j, 0, pidx );
            SourcesModel::SourceType type = SourcesModel::indexType( idx );
            
            if ( type == SourcesModel::DynamicPlaylistSource )
            {
                playlist_ptr p = SourcesModel::indexToDynamicPlaylist( idx );
                if ( playlist.data() == p.data() )
                    return idx;
            }
        }
    }
    
    return QModelIndex();
}


QModelIndex
SourcesModel::collectionToIndex( const Tomahawk::collection_ptr& collection )
{
    for ( int i = 0; i < rowCount(); i++ )
    {
        QModelIndex idx = index( i, 0 );
        SourcesModel::SourceType type = SourcesModel::indexType( idx );
        if ( type == SourcesModel::CollectionSource )
        {
            SourceTreeItem* sti = SourcesModel::indexToTreeItem( idx );
            if ( sti && !sti->source().isNull() && sti->source()->collection().data() == collection.data() )
                return idx;
        }
    }

    return QModelIndex();
}


bool
SourcesModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
    Q_UNUSED( role );
    qDebug() << Q_FUNC_INFO;

    if ( !index.isValid() )
        return false;

    playlist_ptr playlist;
    if ( indexType( index ) == PlaylistSource )
    {
        playlist = indexToPlaylist( index );
    }
    else if ( indexType( index ) == DynamicPlaylistSource )
    {
        playlist = indexToDynamicPlaylist( index ).staticCast< Playlist >();
    }
    
    if ( !playlist.isNull() )
    {
        playlist->rename( value.toString() );
        QStandardItemModel::setData( index, value, Qt::DisplayRole );   
        return true;
    }

    return false;
}


void
SourcesModel::onSourceChanged()
{
    Source* src = qobject_cast< Source* >( sender() );
    
    for ( int i = 0; i < rowCount(); i++ )
    {
        QModelIndex idx = index( i, 0 );

        if ( indexType( idx ) == CollectionSource )
        {
            SourceTreeItem* sti = indexToTreeItem( idx );
            if ( sti )
            {
                if ( sti->source().data() == src )
                {
                    emit dataChanged( idx, idx );
                }
            }
        }
    }
}
