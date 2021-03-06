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

#include "zeroconf.h"

#include <QtPlugin>

const QString
ZeroconfPlugin::name()
{
    return QString( MYNAME );
}

const QString
ZeroconfPlugin::accountName()
{
    return QString();
}

const QString
ZeroconfPlugin::friendlyName()
{
    return QString( "Zeroconf" );
}

bool
ZeroconfPlugin::connectPlugin( bool /*startup*/ )
{
    delete m_zeroconf;
    m_zeroconf = new TomahawkZeroconf( Servent::instance()->port(), this );
    QObject::connect( m_zeroconf, SIGNAL( tomahawkHostFound( QString, int, QString, QString ) ),
                                    SLOT( lanHostFound( QString, int, QString, QString ) ) );

    m_zeroconf->advertise();
    m_isOnline = true;

    foreach( QStringList *currNode, m_cachedNodes )
    {
        QStringList nodeSet = *currNode;
        if ( !Servent::instance()->connectedToSession( nodeSet[3] ) )
            Servent::instance()->connectToPeer( nodeSet[0], nodeSet[1].toInt(), "whitelist", nodeSet[2], nodeSet[3] );

        delete currNode;
    }

    return true;
}

void
ZeroconfPlugin::disconnectPlugin()
{
    m_isOnline = false;

    delete m_zeroconf;
    m_zeroconf = 0;
}

void
ZeroconfPlugin::lanHostFound( const QString& host, int port, const QString& name, const QString& nodeid )
{
    if ( sender() != m_zeroconf )
        return;

    qDebug() << "Found LAN host:" << host << port << nodeid;

    if ( !m_isOnline )
    {
        qDebug() << "Not online, so not connecting.";
        QStringList *nodeSet = new QStringList();
        *nodeSet << host << QString::number( port ) << name << nodeid;
        m_cachedNodes.insert( nodeSet );
        return;
    }
    
    if ( !Servent::instance()->connectedToSession( nodeid ) )
        Servent::instance()->connectToPeer( host, port, "whitelist", name, nodeid );
    else
        qDebug() << "Already connected to" << host;
}

Q_EXPORT_PLUGIN2( sip, ZeroconfPlugin )
