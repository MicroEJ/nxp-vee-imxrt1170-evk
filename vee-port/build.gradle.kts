/*
 * Kotlin
 *
 * Copyright 2026 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

import java.util.*
import javax.xml.parsers.DocumentBuilderFactory
import javax.xml.transform.TransformerFactory
import javax.xml.transform.dom.DOMSource
import javax.xml.transform.stream.StreamResult
import java.io.FileOutputStream

plugins {
    id("com.microej.gradle.veeport") version libs.versions.microej.sdk
}

microej {
    skippedCheckers = "changelog,readme,license"
    additionalFilesDir.set(rootProject.layout.projectDirectory)
}

dependencies {
    microejArchitecture(libs.architecture)

    // Only 3 packs are installed by default: UI, FS and NET.
    // Comment/uncomment the packs depending on your needs.
    // If the UI pack is removed, do not forget to remove the related
    // dependencies and configuration in the Front Panel project.
    microejPack(libs.pack.ui.architecture)
    microejPack(libs.api.event)
    microejPack(libs.pack.fs)
    microejPack(libs.pack.gpio)
    microejPack(libs.pack.gpio.properties)
    microejPack(libs.pack.net)
    microejPack(libs.pack.device)
    microejPack(libs.pack.ecom.wifi)
    microejPack(libs.pack.vg)
    microejPack(libs.pack.serial)
    microejPack(libs.api.microai)

    microejFrontPanel(project(":vee-port:front-panel"))

    microejMock(project(":vee-port:mock"))
    microejMock(libs.jenkins)

    microejTool(project(":vee-port:image-generator"))
}

val cleanVee by tasks.registering(Delete::class) {
    // Load configuration file
    val configurationFile = project.layout.projectDirectory.file("configuration.properties").asFile
    val properties = Properties().apply {
        configurationFile.inputStream().use { fis ->
            load(fis)
        }
    }

    // Get BSP root dir
    val bspRootDir = properties.getProperty("bsp.root.dir").replace("\${project.parent.dir}", "..") + "/"

    // Delete inc/, lib/ directories
    val directories = listOf(
        bspRootDir + properties.getProperty("bsp.microejinc.relative.dir"),
        bspRootDir + properties.getProperty("bsp.microejlib.relative.dir"),
        bspRootDir + properties.getProperty("bsp.microejapp.relative.dir")
    )
    delete(directories)

    // Execute clean script from the BSP
    val microejScriptDir = bspRootDir + properties.getProperty("bsp.microejscript.relative.dir")
    if(org.gradle.internal.os.OperatingSystem.current().isWindows) {
        val scriptFile = project.file("$microejScriptDir/clean.bat")
        exec {
            executable = scriptFile.absolutePath
        }
    } else { // Linux or MacOS
        val scriptFile = project.file("$microejScriptDir/clean.sh")
        exec {
            executable = "bash"
            args(scriptFile.absolutePath)
        }
    }
}

tasks.named("clean") {
    dependsOn(cleanVee)
}

tasks.register("updateIvyDescriptor") {
    doLast {
        val ivyFile = layout.buildDirectory.file("ivy.xml").get().asFile.absolutePath

        val factory = DocumentBuilderFactory.newInstance()
        val builder = factory.newDocumentBuilder()
        val document = builder.parse(ivyFile)
        val publicationsElements = document.getElementsByTagName("publications")
        if (publicationsElements.length == 1) {
            val publications = publicationsElements.item(0)
            val newPublication = document.createElement("artifact")
            newPublication.setAttribute("name", "vee-port")
            newPublication.setAttribute("m:classifier", "notice")
            newPublication.setAttribute("ext", "txt")
            newPublication.setAttribute("type", "text")
            newPublication.setAttribute("conf", "dist")
            publications.appendChild(newPublication)

            val transformerFactory = TransformerFactory.newInstance()
            val transformer = transformerFactory.newTransformer()
            val source = DOMSource(document)
            val result = StreamResult(FileOutputStream(ivyFile))
            transformer.transform(source, result)
        }
    }
}

tasks.getByName("generateIvyDescriptor").finalizedBy("updateIvyDescriptor")

artifacts {
    add("additionalElements", file("../NOTICE.txt")) {
        classifier = "notice"
    }
}