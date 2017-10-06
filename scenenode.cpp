﻿/*
This Source Code Form is subject to the
terms of the Mozilla Public License, v.
2.0. If a copy of the MPL was not
distributed with this file, You can
obtain one at
http://mozilla.org/MPL/2.0/.
*/

#include "stdafx.h"
#include "scenenode.h"

#include "renderer.h"
#include "logs.h"

namespace scene {

// restores content of the node from provded input stream
shape_node &
shape_node::deserialize( cParser &Input, node_data const &Nodedata ) {

    // import common data
    m_name = Nodedata.name;
    m_data.rangesquared_min = Nodedata.range_min * Nodedata.range_min;
    m_data.rangesquared_max = (
        Nodedata.range_max >= 0.0 ?
            Nodedata.range_max * Nodedata.range_max :
            std::numeric_limits<double>::max() );

    std::string token = Input.getToken<std::string>();
    if( token == "material" ) {
        // lighting settings
        token = Input.getToken<std::string>();
        while( token != "endmaterial" ) {

            if( token == "ambient:" ) {
                Input.getTokens( 3 );
                Input
                    >> m_data.lighting.ambient.r
                    >> m_data.lighting.ambient.g
                    >> m_data.lighting.ambient.b;
                m_data.lighting.ambient /= 255.f;
                m_data.lighting.ambient.a = 1.f;
            }
            else if( token == "diffuse:" ) {
                Input.getTokens( 3 );
                Input
                    >> m_data.lighting.diffuse.r
                    >> m_data.lighting.diffuse.g
                    >> m_data.lighting.diffuse.b;
                m_data.lighting.diffuse /= 255.f;
                m_data.lighting.diffuse.a = 1.f;
            }
            else if( token == "specular:" ) {
                Input.getTokens( 3 );
                Input
                    >> m_data.lighting.specular.r
                    >> m_data.lighting.specular.g
                    >> m_data.lighting.specular.b;
                m_data.lighting.specular /= 255.f;
                m_data.lighting.specular.a = 1.f;
            }
            token = Input.getToken<std::string>();
        }
        token = Input.getToken<std::string>();
    }

    // assigned material
    m_data.material = GfxRenderer.Fetch_Material( token );

    // determine way to proceed from the assigned diffuse texture
    // TBT, TODO: add methods to material manager to access these simpler
    auto const texturehandle = (
        m_data.material != null_handle ?
            GfxRenderer.Material( m_data.material ).texture1 :
            null_handle );
    auto const &texture = (
        texturehandle ?
            GfxRenderer.Texture( texturehandle ) :
            opengl_texture() ); // dirty workaround for lack of better api
    bool const clamps = (
        texturehandle ?
            texture.traits.find( 's' ) != std::string::npos :
            false );
    bool const clampt = (
        texturehandle ?
            texture.traits.find( 't' ) != std::string::npos :
            false );

    // remainder of legacy 'problend' system -- geometry assigned a texture with '@' in its name is treated as translucent, opaque otherwise
    if( texturehandle != null_handle ) {
        m_data.translucent = (
            ( ( texture.name.find( '@' ) != std::string::npos )
           && ( true == texture.has_alpha ) ) ?
                true :
                false );
    }
    else {
        m_data.translucent = false;
    }

    // geometry
    enum subtype {
        triangles,
        triangle_strip,
        triangle_fan
    } const nodetype = (
        Nodedata.type == "triangles" ?      triangles :
        Nodedata.type == "triangle_strip" ? triangle_strip :
                                            triangle_fan );
    std::size_t vertexcount{ 0 };
    world_vertex vertex, vertex1, vertex2;
    do {
        Input.getTokens( 8, false );
        Input
            >> vertex.position.x
            >> vertex.position.y
            >> vertex.position.z
            >> vertex.normal.x
            >> vertex.normal.y
            >> vertex.normal.z
            >> vertex.texture.s
            >> vertex.texture.t;
        // clamp texture coordinates if texture wrapping is off
        if( true == clamps ) { vertex.texture.s = clamp( vertex.texture.s, 0.001f, 0.999f ); }
        if( true == clampt ) { vertex.texture.t = clamp( vertex.texture.t, 0.001f, 0.999f ); }
        // convert all data to gl_triangles to allow data merge for matching nodes
        switch( nodetype ) {
            case triangles: {

                     if( vertexcount == 0 ) { vertex1 = vertex; }
                else if( vertexcount == 1 ) { vertex2 = vertex; }
                else if( vertexcount >= 2 ) {
                    if( false == degenerate( vertex1.position, vertex2.position, vertex.position ) ) {
                        m_data.vertices.emplace_back( vertex1 );
                        m_data.vertices.emplace_back( vertex2 );
                        m_data.vertices.emplace_back( vertex );
                    }
                    else {
                        ErrorLog(
                            "Bad geometry: degenerate triangle encountered"
                            + ( m_name != "" ? " in node \"" + m_name + "\"" : "" )
                            + " (vertices: " + to_string( vertex1.position ) + " + " + to_string( vertex2.position ) + " + " + to_string( vertex.position ) + ")" );
                    }
                }
                ++vertexcount;
                if( vertexcount > 2 ) { vertexcount = 0; } // start new triangle if needed
                break;
            }
            case triangle_fan: {

                     if( vertexcount == 0 ) { vertex1 = vertex; }
                else if( vertexcount == 1 ) { vertex2 = vertex; }
                else if( vertexcount >= 2 ) {
                    if( false == degenerate( vertex1.position, vertex2.position, vertex.position ) ) {
                        m_data.vertices.emplace_back( vertex1 );
                        m_data.vertices.emplace_back( vertex2 );
                        m_data.vertices.emplace_back( vertex );
                        vertex2 = vertex;
                    }
                    else {
                        ErrorLog(
                            "Bad geometry: degenerate triangle encountered"
                            + ( m_name != "" ? " in node \"" + m_name + "\"" : "" )
                            + " (vertices: " + to_string( vertex1.position ) + " + " + to_string( vertex2.position ) + " + " + to_string( vertex.position ) + ")" );
                    }
                }
                ++vertexcount;
                break;
            }
            case triangle_strip: {

                     if( vertexcount == 0 ) { vertex1 = vertex; }
                else if( vertexcount == 1 ) { vertex2 = vertex; }
                else if( vertexcount >= 2 ) {
                    if( false == degenerate( vertex1.position, vertex2.position, vertex.position ) ) {
                        // swap order every other triangle, to maintain consistent winding
                        if( vertexcount % 2 == 0 ) {
                            m_data.vertices.emplace_back( vertex1 );
                            m_data.vertices.emplace_back( vertex2 );
                        }
                        else {
                            m_data.vertices.emplace_back( vertex2 );
                            m_data.vertices.emplace_back( vertex1 );
                        }
                        m_data.vertices.emplace_back( vertex );

                        vertex1 = vertex2;
                        vertex2 = vertex;
                    }
                    else {
                        ErrorLog(
                            "Bad geometry: degenerate triangle encountered"
                            + ( m_name != "" ? " in node \"" + m_name + "\"" : "" )
                            + " (vertices: " + to_string( vertex1.position ) + " + " + to_string( vertex2.position ) + " + " + to_string( vertex.position ) + ")" );
                    }
                }
                ++vertexcount;
                break;
            }
            default: { break; }
        }
        token = Input.getToken<std::string>();

    } while( token != "endtri" );

    return *this;
}

// adds content of provided node to already enclosed geometry. returns: true if merge could be performed
bool
shape_node::merge( shape_node &Shape ) {

    if( ( m_data.material != Shape.m_data.material )
     || ( m_data.lighting != Shape.m_data.lighting ) ) {
        // can't merge nodes with different appearance
        return false;
    }
    // add geometry from provided node
    m_data.area.center =
        interpolate(
            m_data.area.center, Shape.m_data.area.center,
            static_cast<float>( Shape.m_data.vertices.size() ) / ( Shape.m_data.vertices.size() + m_data.vertices.size() ) );
    m_data.vertices.insert(
        std::end( m_data.vertices ),
        std::begin( Shape.m_data.vertices ), std::end( Shape.m_data.vertices ) );
    // NOTE: we could recalculate radius with something other than brute force, but it'll do
    compute_radius();

    return true;
}

// generates renderable version of held non-instanced geometry in specified geometry bank
void
shape_node::create_geometry( geometrybank_handle const &Bank ) {

    vertex_array vertices; vertices.reserve( m_data.vertices.size() );

    for( auto const &vertex : m_data.vertices ) {
        vertices.emplace_back(
            vertex.position - m_data.origin,
            vertex.normal,
            vertex.texture );
    }
    m_data.geometry = GfxRenderer.Insert( vertices, Bank, GL_TRIANGLES );
    std::vector<world_vertex>().swap( m_data.vertices ); // hipster shrink_to_fit
}

// calculates shape's bounding radius
void
shape_node::compute_radius() {

    auto squaredradius { 0.0 };
    for( auto const &vertex : m_data.vertices ) {
        squaredradius = std::max(
            squaredradius,
            glm::length2( vertex.position - m_data.area.center ) );
    }
    m_data.area.radius = static_cast<float>( std::sqrt( squaredradius ) );
}


/*
memory_node &
memory_node::deserialize( cParser &Input, node_data const &Nodedata ) {

    // import common data
    m_name = Nodedata.name;

    Input.getTokens( 3 );
    Input
        >> m_data.area.center.x
        >> m_data.area.center.y
        >> m_data.area.center.z;

    TMemCell memorycell( Nodedata.name );
    memorycell.Load( &Input );
}
*/
} // scene

namespace editor {

basic_node::basic_node( scene::node_data const &Nodedata ) :
    m_name( Nodedata.name )
{
    m_rangesquaredmin = Nodedata.range_min * Nodedata.range_min;
    m_rangesquaredmax = (
        Nodedata.range_max >= 0.0 ?
            Nodedata.range_max * Nodedata.range_max :
            std::numeric_limits<double>::max() );
}


} // editor
//---------------------------------------------------------------------------
