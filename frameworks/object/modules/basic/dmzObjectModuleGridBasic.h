#ifndef DMZ_OBJECT_MODULE_GRID_BASIC_DOT_H
#define DMZ_OBJECT_MODULE_GRID_BASIC_DOT_H

#include <dmzObjectModuleGrid.h>
#include <dmzObjectObserverUtil.h>
#include <dmzRuntimeLog.h>
#include <dmzRuntimeObjectType.h>
#include <dmzRuntimePlugin.h>
#include <dmzTypesHashTableHandleTemplate.h>
#include <dmzTypesVector.h>

namespace dmz {

   class ObjectModuleGridBasic :
         public Plugin,
         public ObjectModuleGrid,
         public ObjectObserverUtil {

      public:
         ObjectModuleGridBasic (const PluginInfo &Info, Config &local);
         ~ObjectModuleGridBasic ();

         // Plugin Interface
         virtual void update_plugin_state (
            const PluginStateEnum State,
            const UInt32 Level);

         virtual void discover_plugin (
            const PluginDiscoverEnum Mode,
            const Plugin *PluginPtr);

         // ObjectModuleGrid Interface
         virtual Boolean register_object_observer_grid (ObjectObserverGrid &observer);
         virtual Boolean update_object_observer_grid (ObjectObserverGrid &observer);
         virtual Boolean release_object_observer_grid (ObjectObserverGrid &observer);

         virtual void find_objects (
            const Volume &SearchSpace,
            HandleContainer &objects,
            const ObjectTypeSet *IncludeTypes,
            const ObjectTypeSet *ExcludeTypes);

         // Object Observer Interface
         virtual void create_object (
            const UUID &Identity,
            const Handle ObjectHandle,
            const ObjectType &Type,
            const ObjectLocalityEnum Locality);

         virtual void destroy_object (const UUID &Identity, const Handle ObjectHandle);

         virtual void update_object_position (
            const UUID &Identity,
            const Handle ObjectHandle,
            const Handle AttributeHandle,
            const Vector &Value,
            const Vector *PreviousValue);

      protected:
         struct ObjectStruct {

            const ObjectType Type;
            Vector pos;

            Int32 place;
            ObjectStruct *prev;
            ObjectStruct *next;

            ObjectStruct *node;

            ObjectStruct (const ObjectType &TheType) :
                  Type (TheType),
                  place (-1),
                  prev (0),
                  next (0),
                  node (0) {;}
         };

         struct GridStruct {

            ObjectStruct *objList;

            GridStruct () : objList (0) {;}
         };

         void _init (Config &local);
         Int32 _map_coord (const Int32 X, const Int32 Y);
         void _map_point_to_coord (const Vector &Point, Int32 &x, Int32 &y);
         Int32 _map_point (const Vector &Point);

         Log _log;

         VectorComponentEnum _primaryAxis;
         VectorComponentEnum _secondaryAxis;

         Int32 _xCoordMax;
         Int32 _yCoordMax;

         Vector _minGrid;
         Vector _maxGrid;
         Float64 _xCellSize;
         Float64 _yCellSize;

         GridStruct *_grid;

      private:
         ObjectModuleGridBasic ();
         ObjectModuleGridBasic (const ObjectModuleGridBasic &);
         ObjectModuleGridBasic &operator= (const ObjectModuleGridBasic &);
   };
};


inline dmz::Int32
dmz::ObjectModuleGridBasic::_map_coord (const Int32 X, const Int32 Y) {

   return ((_yCoordMax - 1) * (Y < 0 ? 0 : (Y >= _yCoordMax ? _yCoordMax - 1 : Y))) +
      (X < 0 ? 0 : (X >= _xCoordMax ? _xCoordMax - 1 : X));
}


inline void
dmz::ObjectModuleGridBasic::_map_point_to_coord (
      const Vector &Point,
      Int32 &x,
      Int32 &y) {

   Vector vec (Point - _minGrid);
   x = (Int32)(vec.get (_primaryAxis) / _xCellSize);
   y = (Int32)(vec.get (_secondaryAxis) / _yCellSize);
}


inline dmz::Int32
dmz::ObjectModuleGridBasic::_map_point (const Vector &Point) {

   Int32 x = 0, y = 0;
   _map_point_to_coord (Point, x, y);
   return _map_coord (x, y);
}

#endif // DMZ_OBJECT_MODULE_GRID_BASIC_DOT_H
