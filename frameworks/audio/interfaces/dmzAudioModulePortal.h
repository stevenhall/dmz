#ifndef DMZ_AUDIO_MODULE_PORTAL_DOT_H
#define DMZ_AUDIO_MODULE_PORTAL_DOT_H

#include <dmzRuntimePlugin.h>
#include <dmzRuntimeRTTI.h>
#include <dmzTypesBase.h>


namespace dmz {

   //! \cond
   const char AudioModulePortalInterfaceName[] =  "AudioModulePortalInterface";
   //! \endcond

   class Matrix;
   class Vector;

   class AudioModulePortal {

      public:
         static AudioModulePortal *cast (
            const Plugin *PluginPtr,
            const String &PluginName = "");

         String get_audio_portal_name () const;
         Handle get_audio_portal_handle () const;

         // AudioModulePortal Interface
         virtual Boolean is_master_portal () const = 0;

         virtual void set_view (
            const Vector &Position,
            const Matrix &Orientation,
            const Vector &Velocity) = 0;

         virtual void get_view (
            Vector &position,
            Matrix &orientation,
            Vector &velocity) const = 0;

         virtual void set_mute_state (const Boolean Value) = 0;
         virtual Boolean get_mute_state () const  = 0;

      protected:
         AudioModulePortal (const PluginInfo &Info);
         ~AudioModulePortal ();

      private:
         AudioModulePortal (const AudioModulePortal &);
         AudioModulePortal &operator= (const AudioModulePortal &);
         const PluginInfo &__Info;
   };
};


inline dmz::AudioModulePortal *
dmz::AudioModulePortal::cast (const Plugin *PluginPtr, const String &PluginName) {

   return (AudioModulePortal *)lookup_rtti_interface (
      AudioModulePortalInterfaceName,
      PluginName,
      PluginPtr);
}


inline
dmz::AudioModulePortal::AudioModulePortal (const PluginInfo &Info) :
      __Info (Info) {

   store_rtti_interface (AudioModulePortalInterfaceName, __Info, (void *)this);
}


inline
dmz::AudioModulePortal::~AudioModulePortal () {

   remove_rtti_interface (AudioModulePortalInterfaceName, __Info);
}

dmz::String
dmz::AudioModulePortal::get_audio_portal_name () const { return __Info.get_name (); }


dmz::Handle
dmz::AudioModulePortal::get_audio_portal_handle () const { return __Info.get_handle (); }

#endif //  DMZ_AUDIO_MODULE_PORTAL_DOT_H

