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

#include "databasecommand_loadallplaylists.h"

#include <QSqlQuery>

#include "playlist.h"
#include "databaseimpl.h"

using namespace Tomahawk;


void DatabaseCommand_LoadAllPlaylists::exec( DatabaseImpl* dbi )
{
    TomahawkSqlQuery query = dbi->newquery();

    query.exec( QString( "SELECT guid, title, info, creator, lastmodified, shared, currentrevision "
                         "FROM playlist WHERE source %1 AND dynplaylist = 'false'" )
                   .arg( source()->isLocal() ? "IS NULL" :
                         QString( "= %1" ).arg( source()->id() )
                       ) );

    QList<playlist_ptr> plists;
    while ( query.next() )
    {
        playlist_ptr p( new Playlist( source(),                  //src
                                      query.value(6).toString(), //current rev
                                      query.value(1).toString(), //title
                                      query.value(2).toString(), //info
                                      query.value(3).toString(), //creator
                                      query.value(5).toBool(),   //shared
                                      query.value(4).toInt(),    //lastmod
                                      query.value(0).toString()  //GUID
                                    ) );
        plists.append( p );
    }

    emit done( plists );
}

