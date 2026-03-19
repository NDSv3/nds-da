Implementation Details
======================

This page summarizes the internal architecture of the NDS-DeviceAccess library.

The implementation is split in three layers:

1. NDS-facing device layer (``ndsdaDevice``)
2. Conversion/dispatch layer (``DABridge``)
3. DeviceAccess register layer (``DAAccessor``)

Class Structure And Ownership
-----------------------------

.. mermaid::

   classDiagram
	   direction TB

	   class ndsdaDevice {
		   -std::string deviceName
		   -std::string deviceAlias
		   -std::string deviceMapFile
		   -std::string memoryMapFile
		   -std::string nodeMapFile
		   -std::unique_ptr~DABridge~ daBridge
		   -std::vector~std::shared_ptr~NodeMapPV~~ nodeMapPVs
		   +readPV~T~(pv)
		   +writePV~T~(pv, value, ts)
	   }

	   class DABridge {
		   -std::unique_ptr~DAAccessor~ daAccessor
		   +readPV~T~(NodeMapPV)
		   +writePV~T~(ts, value, NodeMapPV)
	   }

	   class DAAccessor {
		   -ChimeraTK::Device daDevice
		   +readRegister~T~(RegisterElement)
		   +writeRegister~T~(RegisterElement, value)
		   +readOneDRegister~T~(OneDElement)
		   +writeOneDRegister~T~(OneDElement, values)
	   }

	   class NodeMapPV {
		   +std::string name
		   +std::string reg
		   +PVTypeEnum type
		   +size_t nElem
		   +std::string readPV
		   +std::string writePV
		   +std::unique_ptr~MemoryElement~ memoryElement
	   }

	   class MemoryElement {
		   <<abstract>>
		   +std::string name
		   +size_t fracBits
		   +size_t elementWidthBits
		   +bool isSigned
		   +getType() MemoryElementType
	   }

	   class RegisterElement {
		   +std::optional~DARegAccessorVariant~ daAccessor
	   }

	   class OneDElement {
		   +size_t elements
		   +std::optional~DAOneDRegAccessorVariant~ daAccessor
	   }

	   class FileParser {
		   <<namespace>>
		   +getMemoryMapFile(dmap, alias)
		   +parseMemoryMapFile(mmap)
		   +parseNodeMapFile(xml)
	   }

	   class ndsPort {
		   <<external nds::Port>>
	   }

	   class chimeraDevice {
		   <<external ChimeraTK::Device>>
	   }

	   ndsdaDevice *-- DABridge : owns
	   DABridge *-- DAAccessor : owns
	   ndsdaDevice o-- NodeMapPV : keeps list
	   NodeMapPV *-- MemoryElement : owns
	   MemoryElement <|-- RegisterElement
	   MemoryElement <|-- OneDElement
	   ndsdaDevice ..> FileParser : parses files
	   ndsdaDevice ..> ndsPort : publishes PVs
	   DAAccessor ..> chimeraDevice : register access


Layer Interaction (NDS <-> DeviceAccess)
----------------------------------------

.. mermaid::

   flowchart LR
	   subgraph NDS["NDS Layer"]
		   CLIENT["NDS client / control system"]
		   PORT["nds::Port with PVDelegateIn/PVDelegateOut"]
	   end

	   subgraph LIB["NDS-DeviceAccess Library"]
		   DEV["ndsdaDevice\n- loads NodeMap + MemoryMap\n- creates delegates"]
		   BR["DABridge\n- type dispatch\n- scalar/1D/string/bool conversion"]
		   ACC["DAAccessor\n- cached register accessors\n- typed read/write"]
		   NMPV["NodeMapPV + MemoryElement\nRegisterElement | OneDElement"]
		   PARSER["FileParser\nparse .dmap/.mmap/.xml"]
	   end

	   subgraph DA["ChimeraTK DeviceAccess"]
		   CHDEV["ChimeraTK::Device"]
		   REGS["Hardware registers"]
	   end

	   CLIENT <--> PORT
	   PORT <--> DEV
	   DEV --> PARSER
	   DEV --> NMPV
	   DEV <--> BR
	   BR <--> ACC
	   ACC <--> CHDEV
	   CHDEV <--> REGS


Operational Notes
-----------------

* ``ndsdaDevice`` is intended as a base class for concrete drivers.
* Derived drivers add custom PVs/nodes and then call ``rootNode.initialize(this, factory)``.
* ``NodeMapPV`` couples NDS names with DeviceAccess register definitions.
* ``DABridge`` performs runtime type dispatch from ``PVTypeEnum`` to templated read/write code.
* ``DAAccessor`` caches ``ChimeraTK`` accessors in ``RegisterElement``/``OneDElement`` for reuse.