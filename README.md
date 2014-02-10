open-turbidimeter-project
=========================

The high cost of equipment to monitor water quality often puts valuable health tools out of reach for many communities in developing countries. Our low-cost device measures the turbidity, or "cloudiness", of water due to suspended particles, and can detect potentially dangerous concentrations of dirt in water -- even when they are invisible to the human eye! Our device integrates with a remote data-acquisition system to enable record keeping and real-time observation of water quality in rivers, wells, and treatment plants.

Rationale
=========

Turbidity refers to the dirtiness or cloudiness of a water sample. Technicians at water treatment plants regularly measure water turbidity before and after treatment. It’s a legal requirement in many countries, and one indicator of whether treated water is safe for human consumption.

The unaided eye can distinguish cloudy water from clear water, but even visibly clear samples of water can have dangerous levels of turbidity. The standard tool for measuring turbidity (a turbidimeter) is a complex piece of equipment. Handheld units capable of analyzing a single sample vial cost several hundred dollars; automated units capable of intermittently analyzing samples from a column of flowing water can cost several thousand. An affordable, open-source solution for turbidity sampling would be a major step towards assuring the quality and sustainability of programs that seek to expand global access to treated drinking water. Partly completed open-source turbidimeter projects can be found online, apparently abandoned.



Design Goals
============

(1) Turbidimeter costs less than $100 (for a production model),

(2*) Reads samples with an accuracy of 0.05 NTU in the sample range 0-5 NTU, and 0.2 NTU in the range of 5-20 NTU,

(3*) Reads samples with an accuracy of 5% in the range 20-1000 NTU,

(4) Can be operated manually, or in automated mode for continuous sampling,

(5) Transmits data to an Android phone, via USB connection or Bluetooth,

(6) Is powered by 2-4 AA batteries,

An open-source turbidimeter fulfilling the design goals listed above will not quite meet the strict technical requirements of the EPA, but will nonetheless provide a huge improvement over the status-quo of rural and small-scale water treatment monitoring, at a realistic cost.
